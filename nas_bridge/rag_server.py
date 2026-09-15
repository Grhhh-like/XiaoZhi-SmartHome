# -*- coding: utf-8 -*-
"""
rag_server.py - 家庭文档 RAG 检索问答服务
小智·智家 XiaoZhi SmartHome

把 NAS/家庭目录中的文档（PDF/DOCX/TXT/MD）切片、向量化，
提供"语音问文档"能力。数据全程本地处理。

流程: 文档解析 → 分块 → 向量化(本地嵌入) → 向量检索 → 拼接上下文 → 调用大模型生成
"""
import argparse
import glob
import json
import logging
import os
import re

from flask import Flask, request, jsonify

log = logging.getLogger("rag_server")

# 轻量本地向量实现（演示/教学版）
# 生产环境可替换为 sentence-transformers / bge 模型
try:
    import numpy as np
    HAS_NUMPY = True
except ImportError:
    HAS_NUMPY = False

# ---------------- 文档解析 ----------------
def extract_text(path):
    ext = os.path.splitext(path)[1].lower()
    if ext == ".txt" or ext == ".md":
        with open(path, "r", encoding="utf-8", errors="ignore") as f:
            return f.read()
    if ext == ".pdf":
        try:
            from pypdf import PdfReader
            reader = PdfReader(path)
            return "\n".join(page.extract_text() or "" for page in reader.pages)
        except ImportError:
            return ""
    if ext == ".docx":
        try:
            from docx import Document
            doc = Document(path)
            return "\n".join(p.text for p in doc.paragraphs)
        except ImportError:
            return ""
    return ""


def chunk_text(text, size=500, overlap=50):
    """按段落/定长切片，保留重叠避免上下文断裂"""
    chunks = []
    paras = re.split(r"\n\s*\n", text)
    buf = ""
    for p in paras:
        if len(buf) + len(p) > size and buf:
            chunks.append(buf)
            buf = buf[-overlap:] + p
        else:
            buf += "\n" + p
    if buf:
        chunks.append(buf)
    return chunks


# ---------------- 轻量向量（字面特征哈希，教学演示） ----------------
def embed(text):
    """演示用特征向量：词袋 + 归一化。生产替换为语义向量模型"""
    vec = {}
    for tok in re.findall(r"[\u4e00-\u9fff]{2,}|[a-zA-Z0-9_]{2,}", text.lower()):
        vec[tok] = vec.get(tok, 0) + 1
    if HAS_NUMPY:
        keys = sorted(vec.keys())
        v = np.zeros(4096, dtype=np.float32)
        for k in keys:
            h = abs(hash(k)) % 4096
            v[h] += vec[k]
        norm = np.linalg.norm(v)
        return v / norm if norm > 0 else v
    return vec


def cos_sim(a, b):
    if HAS_NUMPY:
        return float(np.dot(a, b))
    inter = set(a) & set(b)
    if not inter:
        return 0.0
    import math
    na = math.sqrt(sum(v * v for v in a.values()))
    nb = math.sqrt(sum(v * v for v in b.values()))
    if na == 0 or nb == 0:
        return 0.0
    return sum(min(a[t], b[t]) for t in inter) / (na * nb)


# ---------------- 索引 ----------------
class DocIndex:
    def __init__(self, path):
        self.path = path
        self.chunks = []   # [(doc, chunk_text, vec)]

    def build(self, docs_dir):
        self.chunks = []
        for f in sorted(glob.glob(os.path.join(docs_dir, "**", "*"), recursive=True)):
            if not os.path.isfile(f):
                continue
            text = extract_text(f)
            if not text:
                continue
            for c in chunk_text(text):
                self.chunks.append((os.path.basename(f), c, embed(c)))
            log.info("indexed: %s (%d chars)", os.path.basename(f), len(text))
        self.save()
        log.info("total chunks: %d", len(self.chunks))

    def save(self):
        os.makedirs(self.path, exist_ok=True)
        with open(os.path.join(self.path, "index.json"), "w", encoding="utf-8") as f:
            json.dump([(d, c) for d, c, _ in self.chunks], f, ensure_ascii=False)

    def load(self):
        idx = os.path.join(self.path, "index.json")
        if not os.path.exists(idx):
            return
        with open(idx, "r", encoding="utf-8") as f:
            data = json.load(f)
        self.chunks = [(d, c, embed(c)) for d, c in data]
        log.info("loaded %d chunks from %s", len(self.chunks), self.path)

    def search(self, question, top_k=3):
        qv = embed(question)
        scored = [(cos_sim(qv, vec), doc, chunk) for doc, chunk, vec in self.chunks]
        scored.sort(key=lambda x: -x[0])
        return scored[:top_k]


# ---------------- 服务 ----------------
app = Flask(__name__)
index = DocIndex("./index")

@app.post("/query")
def api_query():
    body = request.get_json(force=True)
    question = body.get("question", "")
    if not question:
        return jsonify({"answer": "请输入问题"})

    hits = index.search(question)
    if not hits:
        return jsonify({"answer": "在家庭文档库中未找到相关内容。"})

    context = "\n\n".join(f"[{doc}]\n{chunk}" for score, doc, chunk in hits)
    # 拼接提示词（演示：直接返回最相关片段 + 调用云端大模型生成完整答案）
    answer = _generate_answer(question, context)
    return jsonify({"answer": answer, "sources": [h[1] for h in hits]})


def _generate_answer(question, context):
    """生产实现：调用豆包/Qwen/DeepSeek API，prompt 注入检索上下文"""
    # 演示：本地返回最相关片段摘要
    first = context.split("\n\n")[0]
    if "[" in first and "]" in first:
        doc, content = first[1:].split("]", 1)
        return f"根据文档《{doc}》：{content.strip()[:200]}"
    return first[:200]


@app.get("/health")
def api_health():
    return jsonify({"ok": True, "chunks": len(index.chunks)})


def main():
    logging.basicConfig(level=logging.INFO,
                        format="%(asctime)s [%(name)s] %(levelname)s %(message)s")

    parser = argparse.ArgumentParser(description="XiaoZhi SmartHome RAG Server")
    parser.add_argument("--index", default="./index")
    parser.add_argument("--docs", default=None, help="文档目录，传入则重建索引")
    parser.add_argument("--http-port", type=int, default=8081)
    args = parser.parse_args()

    global index
    index = DocIndex(args.index)
    if args.docs:
        index.build(args.docs)
    else:
        index.load()

    app.run(host="0.0.0.0", port=args.http_port)

if __name__ == "__main__":
    main()
