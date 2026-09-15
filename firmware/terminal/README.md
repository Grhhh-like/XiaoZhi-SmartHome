# 小智终端二次开发固件

> 基于 [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) 的二次开发模块，新增：
> **MCP 智能家居工具集 + 四通道控制（红外/433/MQTT/ESP-NOW）+ 传感器融合 + 主动智能引擎 + 离线降级**

## 集成方式

本目录是 **增量模块**，需要与 xiaozhi-esp32 官方工程合并编译：

```bash
git clone https://github.com/78/xiaozhi-esp32
cd xiaozhi-esp32
# 将本仓库 main/ 下文件复制进工程 main/ 目录（新增模块，不覆盖官方文件）
# 在官方 CMakeLists.txt 的 SRCS 中追加本模块源文件
idf.py set-target esp32s3
idf.py menuconfig   # 按官方文档配置外设(麦/喇叭/屏)与服务器
idf.py build flash monitor
```

## 模块清单

| 文件 | 功能 |
|---|---|
| `main.c` | 入口：初始化各模块、注册 MCP 工具 |
| `mcp_tools.c/h` | MCP 工具集：smart_home_control / ir_send / rf_send / query_sensor / nas_query / scene_execute |
| `ir_remote.c/h` | 红外发射（NEC 码 + RAW 码，38kHz） |
| `rf433.c/h` | 433MHz 射频发射（FS1000A） |
| `mqtt_bridge.c/h` | MQTT 桥（订阅/发布设备状态） |
| `espnow_mesh.c/h` | ESP-NOW 主节点（节点发现/指令下发/状态回读） |
| `sensor_fusion.c/h` | 多传感器融合（环境决策） |
| `proactive.c/h` | 主动智能引擎（规则+习惯学习+主动播报） |
| `offline_cmd.c/h` | 离线命令词引擎（断网降级） |
| `lcd_ui.c/h` | 圆屏表情 UI（听/想/说/执行状态） |

## 引脚定义（默认）

| 功能 | GPIO |
|---|---|
| 红外发射 | GPIO17 |
| 433 发射 | GPIO18 |
| WS2812 灯带 | GPIO48 |
| I2S 麦克风 | 官方默认 |
| I2S 喇叭 | 官方默认 |
| 圆屏 SPI | 官方默认 |

> 按实际开发板修改 `main.c` 顶部 `PIN_*` 宏。

## 配置

`main.c` 顶部配置：

```c
#define MQTT_BROKER_URI   "mqtt://192.168.1.10:1883"
#define MQTT_USER         "smart_home"
#define MQTT_PASS         "change_me"
#define MASTER_MAC        {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX}  // 节点目标MAC(广播可省略)
```

## 协议说明

- ESP-NOW 帧格式见 `espnow_mesh.h`
- MQTT 消息格式见 `mqtt_bridge.h`
- 离线命令词表见 `offline_cmd.h`
