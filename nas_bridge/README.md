# NAS 知识中枢桥（nas_bridge）

把小智 AI 语音机器人接到家庭 NAS，让小智成为**私有数据知识中枢**：
日程、文档、待办、照片全部可"用嘴问、用嘴记"，数据不出家庭。

## 架构

```
小智终端(语音) 
   → 云端大模型(MCP工具调用: nas_query)
   → HTTP(局域网) → bridge.py (运行在NAS/树莓派)
   → NAS API: Synology Calendar / File Station / Photos / 自建文档库
   → RAG 检索 → 摘要返回 → TTS 播报
```

## 功能

| 能力 | 接口 | 一句话示例 |
|---|---|---|
| 日程播报 | Synology Calendar API / CalDAV | "明天上午有什么安排" |
| 文档问答 | RAG（向量检索 NAS 文档） | "上个月总结里写了什么" |
| 找文件 | File Station API | "帮我找一下发票文件" |
| 录音归档 | 文件写入 API | "把会议记录存到 NAS" |
| 待办管理 | Calendar/任务 API | "提醒我周五交房租" |

## 启动

```bash
pip install -r requirements.txt
python bridge.py --nas-type synology --host 192.168.1.10 \
    --user admin --password '***' --rag-index ./index
```

### 配置项

| 参数 | 说明 |
|---|---|
| `--nas-type` | synology / qnap / generic |
| `--host` | NAS 局域网地址 |
| `--user/--password` | NAS 账号 |
| `--rag-index` | RAG 向量索引目录 |
| `--http-port` | 服务端口（默认 8080） |

## RAG 文档问答

`rag_server.py` 提供独立的 RAG 服务：

```bash
# 1. 建立索引（扫描家庭文档目录）
python rag_server.py --index ./index --docs ./家庭文档 --build

# 2. 启动问答服务
python rag_server.py --index ./index --http-port 8081

# 3. 测试
curl -X POST http://127.0.0.1:8081/query -d '{"question":"上个月总结里写了什么"}'
```

支持格式：PDF / DOCX / TXT / MD。

## 安全

- 仅监听局域网，防火墙限制外网
- NAS 凭据通过环境变量传入（不写入代码/仓库）
- 个人数据全程留在 NAS，云端大模型只拿到检索摘要
