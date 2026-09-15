# 小智·智家 XiaoZhi SmartHome

> **AI 语音驱动的万物互联家庭管家体系** —— 基于小智 AI（xiaozhi-esp32）的开源智能家居控制系统

![license](https://img.shields.io/badge/license-MIT-green)
![platform](https://img.shields.io/badge/platform-ESP32%20%7C%20ESP32--S3%20%7C%20ESP32--C3-blue)
![protocol](https://img.shields.io/badge/protocol-MCP%20%7C%20WiFi%20%7C%20BLE%20%7C%20MQTT-blue)
![ai](https://img.shields.io/badge/AI-Qwen%20%7C%20DeepSeek%20%7C%20Doubao-purple)

---

## 一句话介绍

**「小智·智家」** 是一套以 AI 语音机器人为核心的 **主动化、互动性、智能化** 家庭智能控制系统：让小智听得懂、说得出、想得到、做得到——不仅能"听话"控制全屋设备，还能**主动感知环境、主动提醒家人、主动执行场景**，并通过 NAS 联动成为家庭私有知识中枢。

## 体系全景

```
┌────────────────────────────────────────────────────────────────┐
│  应用层     手机APP · 网页看板 · 场景模式 · 状态监控              │
├────────────────────────────────────────────────────────────────┤
│  云平台     小智AI服务器(大模型) · MQTT Broker · NAS私有云       │
├────────────────────────────────────────────────────────────────┤
│  AI决策层   流式ASR → 大模型(豆包/Qwen/DeepSeek) → 意图理解      │
│             → MCP工具调用 → 生成控制指令 · RAG知识检索          │
├────────────────────────────────────────────────────────────────┤
│  控制中枢   小智终端(MCP工具) + WiFi联网(MQTT/HTTP) + 蓝牙BLE   │
│             + 离线应急命令（参考米家智能家居模式）               │
├────────────────────────────────────────────────────────────────┤
│  执行层     智能灯 · 智能插座 · 空调 · 窗帘电机 ·                 │
│             环境传感器节点(BLE) · 摄像头联动 · 米家/HA生态       │
└────────────────────────────────────────────────────────────────┘
```

## 核心特性

### 🎯 主动化（Proactive）
- **习惯学习引擎**：观察家庭作息规律，主动提醒（"快十点了，该睡觉了"）
- **异常主动报告**：不等你问，主动播报（"厨房温度偏高，疑似忘关火"）
- **日程晨报**：早上主动播报天气+日程+路况（NAS 日历联动）
- **场景自动触发**：回家自动开灯、离家自动关灯关空调

### 🗣️ 互动性（Interactive）
- **多轮自然对话**：流式 ASR + LLM + TTS，极低延迟
- **状态回读确认**：控制后回读设备状态并语音确认（"客厅灯已打开，亮度30%"）
- **声纹识别**：区分家庭成员，个性化响应
- **视觉×语音联动**：摄像头识别"谁来了"、手势控制

### 🧠 智能化（Intelligent）
- **MCP 工具链**：为大模型注册"开关灯/调空调/拉窗帘/查状态"工具，意图理解+参数抽取
- **多传感器融合**：温湿度/空气质量/光照/人体存在，环境感知决策
- **场景联动引擎**："执行睡眠模式" → 自动关灯+调暗+关窗帘+空调26度
- **离线降级**：断网时本地命令词控制核心设备

### 🏠 万物互联（IoT）
- **双通道统一控制（参考米家模式）**：WiFi联网(MQTT/HTTP) 主通道 + 蓝牙BLE 本地通道，一套语音指令覆盖全屋设备
- **自制智能节点**：BLE 组网的智能灯/窗帘/插座/传感器节点，低功耗
- **NAS 知识中枢**：日程播报、文档问答、录音归档、待办管理（RAG）
- **米家/Home Assistant 桥接**：接入上千种主流智能设备生态

## 仓库结构

```
XiaoZhi-SmartHome/
├── docs/                     # 设计文档、部署文档、演示场景、论文大纲
├── hardware/                 # 嘉立创EDA工程、BOM物料清单
├── firmware/
│   ├── terminal/             # 小智终端二次开发（MCP工具/BLE网关/WiFi联网/主动引擎）
│   └── node/                 # 自制智能节点（灯/窗帘/插座/传感器，BLE从机）
├── gateway/                  # 控制中枢网关（Python，MQTT桥+场景引擎+设备管理）
├── nas_bridge/               # NAS知识中枢（日历API/文件搜索/RAG）
├── app/                      # 网页看板
└── scripts/                  # 工程生成脚本、GitHub上传脚本
```

## 快速开始

1. **准备硬件**：见 [hardware/BOM.csv](hardware/BOM.csv) 与 [hardware/README.md](hardware/README.md)
2. **编译终端固件**：见 [firmware/terminal/README.md](firmware/terminal/README.md)
3. **编译智能节点**：见 [firmware/node/README.md](firmware/node/README.md)
4. **部署网关**：见 [gateway/README.md](gateway/README.md)
5. **接入 NAS**：见 [nas_bridge/README.md](nas_bridge/README.md)
6. **查看看板**：浏览器打开 [app/dashboard.html](app/dashboard.html)

> 详细体系设计见 [docs/design.md](docs/design.md)，部署步骤见 [docs/assembly.md](docs/assembly.md)

## 演示场景

| 场景 | 你说 | 小智做什么 |
|---|---|---|
| 灯光控制 | "把客厅灯调到30%" | BLE 控制灯节点调光 |
| 空调控制 | "空调调到26度" | WiFi联网控制空调(米家/HA) |
| 睡眠模式 | "执行睡眠模式" | 关灯+调暗+关窗帘+空调26度 |
| 环境感知 | "现在室温多少" | 传感器节点回读并播报 |
| 日程查询 | "明天上午有什么安排" | NAS 日历读取并播报 |
| 主动提醒 | （自动） | "厨房温度偏高，疑似忘关火" |
| 离线应急 | 断网时"打开客厅灯" | 本地BLE直连控制核心设备 |

## 技术栈

| 层 | 技术 |
|---|---|
| 语音交互 | xiaozhi-esp32（离线唤醒/声纹/流式ASR-LLM-TTS） |
| 大模型 | 豆包 / Qwen / DeepSeek（MCP 协议） |
| 设备控制 | MCP 工具 + WiFi(MQTT/HTTP) + BLE（参考米家模式） |
| 终端硬件 | ESP32-S3（主控）+ 双麦 + 喇叭 + 圆屏 |
| 节点硬件 | ESP32-C3（低功耗 BLE 从机） |
| 网关 | Python（MQTT Broker 桥接、场景引擎、设备管理） |
| NAS 联动 | 群晖/威联通 API + RAG 向量检索 |

## 致谢

- [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32)：本项目语音交互核心，MIT 协议
- 嘉立创 EDA & 立创开源硬件平台（星火计划 2026）

## License

MIT © Grhhh-like
