# 代码审查报告（Code Review）

> 审查日期：2026-09-16 · 范围：全仓固件 C / 网关 / NAS / 脚本
> 结论：**发现 12 类问题，已修复 11 类，1 类标记为已知限制**

---

## 一、已修复的 Bug（按严重度排序）

### 🔴 严重（会导致崩溃/数据错误）

| # | 文件 | 问题 | 修复 |
|---|---|---|---|
| 1 | `firmware/terminal/main/lcd_ui.c` | `s_face` 是 `const char*` 指向字符串字面量，`snprintf(s_face,...)` 写入只读内存 → **运行崩溃** | 改为 `static char s_face[16]` |
| 2 | `firmware/terminal/main/sensor_fusion.c` | 使用 `esp_timer_get_time()` 但缺少 `#include "esp_timer.h"` → **编译失败** | 补头文件 |
| 3 | `firmware/terminal/main/mqtt_bridge.c` | 使用 `esp_random()` 但缺少 `#include "esp_random.h"`（ESP-IDF 5.x 需要显式包含）→ **编译失败** | 补头文件 |
| 4 | `firmware/terminal/main/ble_gateway.c` | 新节点 `conn_handle` 经 memset 为 0（0 是 NimBLE 有效连接句柄），`send_ctrl` 会把未连接节点误判为已连接 → **误发指令/崩溃风险** | `node_alloc` 中显式置 `0xFFFF`（BLE_HS_CONN_HANDLE_NONE） |
| 5 | `firmware/node/sensor_node/main/main.c` | I2C 读失败不检查返回值，`buf` 未初始化 → **上报垃圾数据** | 全部检查返回值，失败返回安全默认值并告警 |

### 🟠 中等（逻辑错误）

| # | 文件 | 问题 | 修复 |
|---|---|---|---|
| 6 | `firmware/node/curtain_node/main/main.c` | 碰到限位 `break` 后仍执行 `s_position = target`，**实测位置被覆盖**（状态不一致）；`value` 为负转 uint8_t 得 255 → 行程计算错误 | 增加 `hit_limit` 标志，限位结束时保留实测位置；负值钳制为 0 |
| 7 | `firmware/terminal/main/proactive.c` | `movie` 场景匹配成功但无执行代码，返回成功却什么都没做 | 补全 movie（灯光 15% + 关窗帘）；未注册场景返回 `-1` |
| 8 | `firmware/terminal/main/mcp_tools.c` | BLE 控制返回值被忽略，节点离线时仍向用户播报"灯已打开" → **误导用户** | 检查 `ble_*_ctrl` 返回值，离线时返回失败并播报"节点离线" |
| 9 | `firmware/terminal/main/offline_cmd.c` | 词条匹配顺序缺陷："打开卧室插座"会先命中"打开插座"→ 误控客厅插座 | 具体词条前置 + 补充卧室/厨房灯、卧室插座词条；日志参数 `e->cmd` 误传为 `e->value` 一并修正 |
| 10 | `nas_bridge/bridge.py` | `urllib.parse` 仅在 `main()` 内 import，用 gunicorn 直接 import app 时 `NameError` | 提升到模块顶层 |
| 11 | `nas_bridge/rag_server.py` | `hash()` 对 str 随机盐化 → 索引向量跨进程不可复现 | 改用 `hashlib.md5` 确定性哈希 |

### 🟡 轻微（健壮性）

| # | 文件 | 问题 | 修复 |
|---|---|---|---|
| 12 | `gateway/gateway.py` | paho-mqtt 2.x 弃用 `mqtt.Client()` 旧 API；场景 YAML 缺字段 KeyError | 兼容 1.x/2.x 两种构造；`execute_scene` 防御缺字段 |
| 13 | `firmware/terminal/main/main.c` | `sensor_fusion_init_placeholder()` 未声明即调用（隐式声明警告） | 改名 `sensor_fusion_init()` 移入 sensor_fusion.c/h 正式声明 |

---

## 二、已知限制（设计取舍，非缺陷）

| 项 | 说明 |
|---|---|
| 固件未在真实 ESP-IDF 工具链编译 | 无 xtensa 工具链环境；上述修复均为静态审查。落地时执行 `idf.py build` 验证（见 assembly.md 2.1） |
| `motion.c` 的 PID/里程计 | 为骨架 + 协议约定（TODO），需按实际底盘标定：轮径、脉冲数、PID 参数 |
| `ble_gateway.c` NimBLE 回调 | 为协议骨架（TODO），需按官方文档补扫描/连接/GATT 回调 |
| `curtain_node` 步进序列 | `seq_idx` 跨调用保持，若中途 stop 再启动可能从非零相位继续（步进 8 拍序列可容忍） |
| `proactive.c` 习惯学习 | 演示版基于时间窗口，完整版需统计 + 异常检测 |
| `rag_server.py` 向量化 | 词袋哈希教学版，生产替换为 sentence-transformers / bge |

---

## 三、验证方式与覆盖范围

| 验证 | 结果 |
|---|---|
| 全部 `.py` 语法编译（py_compile） | ✅ 通过 |
| `gateway/scenes.yaml` 解析 | ✅ sleep/home/away/movie 4 场景 |
| 全部 `.json` 解析 | ✅ 通过 |
| C 代码 | 静态人工审查（无工具链）；修复点均有注释说明 |
