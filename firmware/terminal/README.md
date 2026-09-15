# 小智终端二次开发固件

> 基于 [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) 的二次开发模块，新增：
> **MCP 智能家居工具集 + WiFi联网(BLE 双通道控制（参考米家模式）+ 传感器融合 + 主动智能引擎 + 离线降级**

## 集成方式

本目录是 **增量模块**，需要与 xiaozhi-esp32 官方工程合并编译：

```bash
git clone https://github.com/78/xiaozhi-esp32
cd xiaozhi-esp32
# 将本仓库 main/ 下文件复制进工程 main/ 目录（新增模块，不覆盖官方文件）
# 在官方 CMakeLists.txt 的 SRCS 中追加本模块源文件
idf.py set-target esp32s3
idf.py menuconfig   # 按官方文档配置外设(麦/喇叭/屏)与服务器；启用蓝牙(BLE)支持
idf.py build flash monitor
```

## 模块清单

| 文件 | 功能 |
|---|---|
| `main.c` | 入口：初始化各模块、注册 MCP 工具 |
| `mcp_tools.c/h` | MCP 工具集：smart_home_control / robot_move / query_sensor / nas_query / scene_execute（WiFi+BLE 双通道路由） |
| `motion.c/h` | 移动底盘：双 N20 电机差速 + 编码器 PID + 里程计 + 避障 |
| `ble_gateway.c/h` | BLE 网关（Central）：扫描 XZ-* 节点、GATT 连接、控制/状态回读 |
| `mqtt_bridge.c/h` | WiFi 联网 MQTT 桥（米家/Home Assistant 生态、自建设备） |
| `sensor_fusion.c/h` | 多传感器融合（环境决策） |
| `proactive.c/h` | 主动智能引擎（规则+习惯学习+主动播报） |
| `offline_cmd.c/h` | 离线命令词引擎（断网时 BLE 本地降级控制） |
| `lcd_ui.c/h` | 圆屏表情 UI（听/想/说/执行状态） |

## 控制通道（参考米家智能家居模式）

| 通道 | 覆盖设备 | 说明 |
|---|---|---|
| WiFi 联网（MQTT/HTTP） | 米家生态 / Home Assistant / 自建 MQTT 设备 | 主通道，云端与局域网统一管理 |
| 蓝牙 BLE | 自制智能节点（灯/窗帘/插座/传感器） | 本地直连，低功耗，无需路由器 |
| 离线命令词 + BLE | 断网时核心设备 | 本地兜底 |

## 引脚定义（默认）

| 功能 | GPIO |
|---|---|
| WS2812 灯带 | GPIO48 |
| I2S 麦克风 | 官方默认 |
| I2S 喇叭 | 官方默认 |
| 圆屏 SPI | 官方默认 |

> 按实际开发板修改。

## 配置

`mqtt_bridge.c` 顶部配置：

```c
#define MQTT_BROKER_URI   "mqtt://192.168.1.10:1883"
#define MQTT_USER         "smart_home"
#define MQTT_PASS         "change_me"
```

## 协议说明

- BLE GATT 协议（UUID/帧格式）见 `ble_gateway.h` 与 `firmware/node/ble_node/`
- MQTT 消息格式见 `mqtt_bridge.h`
- 离线命令词表见 `offline_cmd.h`
