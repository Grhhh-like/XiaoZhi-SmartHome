# -*- coding: utf-8 -*-
"""
bridge.py - NAS 知识中枢桥
小智·智家 XiaoZhi SmartHome

向小智终端/云端大模型提供统一的 HTTP 查询接口，
后端对接群晖(Synology)/威联通(QNAP) NAS 的日历、文件、待办 API，
以及本仓库 rag_server.py 的 RAG 文档问答。

接口：
  POST /query   {"scope":"calendar|docs|todo|files", "query":"..."}
"""
import argparse
import json
import logging
import os
import urllib.request

from flask import Flask, request, jsonify

log = logging.getLogger("nas_bridge")

# ---------------- NAS 配置 ----------------
NAS_TYPE = "synology"
NAS_HOST = "192.168.1.10"
NAS_USER = ""
NAS_PASS = ""
RAG_URL = "http://127.0.0.1:8081/query"


# ---------------- 群晖 API 封装 ----------------
def _synology_login():
    """获取群晖 API 会话 sid（API: auth.cgi）"""
    url = (f"http://{NAS_HOST}:5000/webapi/auth.cgi"
           f"?api=SYNO.API.Auth&version=6&method=login"
           f"&account={urllib.parse.quote(NAS_USER)}"
           f"&passwd={urllib.parse.quote(NAS_PASS)}"
           f"&format=sid")
    with urllib.request.urlopen(url, timeout=5) as resp:
        data = json.loads(resp.read().decode())
    if data.get("success"):
        return data["data"]["sid"]
    log.warning("Synology login failed: %s", data.get("error"))
    return None


def _synology_calendar(sid, query):
    """查询群晖日历日程（Calendar API: SYNO.Calendar.Event）"""
    url = (f"http://{NAS_HOST}:5000/webapi/entry.cgi"
           f"?api=SYNO.Calendar.Event&version=2&method=list"
           f"&_sid={sid}&limit=10&keyword={urllib.parse.quote(query)}")
    with urllib.request.urlopen(url, timeout=5) as resp:
        data = json.loads(resp.read().decode())
    events = data.get("data", {}).get("events", [])
    if not events:
        return "今天/近期没有找到相关日程。"
    lines = [f"{e.get('title','')} {e.get('start_time','')}" for e in events]
    return "；".join(lines)


def _synology_files(sid, query):
    """搜索群晖文件（File Station API: SYNO.FileStation.Search）"""
    url = (f"http://{NAS_HOST}:5000/webapi/entry.cgi"
           f"?api=SYNO.FileStation.Search&version=2&method=start"
           f"&_sid={sid}&folder_path=%5B%22%2F%22%5D&pattern={urllib.parse.quote(query)}"
           f"&limit=5")
    with urllib.request.urlopen(url, timeout=5) as resp:
        data = json.loads(resp.read().decode())
    files = data.get("data", {}).get("files", [])
    if not files:
        return "没有找到相关文件。"
    return "；".join(f.get("path", "") for f in files)


# ---------------- 查询分发 ----------------
def handle_query(scope, query):
    sid = _synology_login()
    if not sid:
        return "NAS 连接失败，请检查 NAS 是否在线。"

    if scope == "calendar":
        return _synology_calendar(sid, query)
    if scope == "files":
        return _synology_files(sid, query)
    if scope == "docs":
        # 文档问答走 RAG
        try:
            req = urllib.request.Request(
                RAG_URL, data=json.dumps({"question": query}).encode(),
                headers={"Content-Type": "application/json"})
            with urllib.request.urlopen(req, timeout=30) as resp:
                return json.loads(resp.read().decode()).get("answer", "未找到相关内容。")
        except Exception as e:
            log.warning("RAG query failed: %s", e)
            return "文档检索服务未就绪。"
    if scope == "todo":
        return "待办管理功能开发中，已记录需求。"
    return f"未知查询范围: {scope}"


# ---------------- HTTP 服务 ----------------
app = Flask(__name__)

@app.post("/query")
def api_query():
    body = request.get_json(force=True)
    scope = body.get("scope", "calendar")
    query = body.get("query", "")
    log.info("query: scope=%s q=%s", scope, query)
    return jsonify({"ok": True, "answer": handle_query(scope, query)})

@app.get("/health")
def api_health():
    return jsonify({"ok": True, "nas_type": NAS_TYPE})


def main():
    logging.basicConfig(level=logging.INFO,
                        format="%(asctime)s [%(name)s] %(levelname)s %(message)s")

    global NAS_TYPE, NAS_HOST, NAS_USER, NAS_PASS, RAG_URL
    import urllib.parse  # noqa: F401 (在函数内使用)

    parser = argparse.ArgumentParser(description="XiaoZhi SmartHome NAS Bridge")
    parser.add_argument("--nas-type", default="synology")
    parser.add_argument("--host", default="192.168.1.10")
    parser.add_argument("--user", default=os.environ.get("NAS_USER", ""))
    parser.add_argument("--password", default=os.environ.get("NAS_PASS", ""))
    parser.add_argument("--rag-url", default="http://127.0.0.1:8081/query")
    parser.add_argument("--http-port", type=int, default=8080)
    args = parser.parse_args()

    NAS_TYPE = args.nas_type
    NAS_HOST = args.host
    NAS_USER = args.user
    NAS_PASS = args.password
    RAG_URL = args.rag_url

    app.run(host="0.0.0.0", port=args.http_port)

if __name__ == "__main__":
    main()
