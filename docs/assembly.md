# 组装与部署文档

> 从零搭建「小智·智家」系统。分五步：硬件准备 → 终端固件 → 节点固件 → 网关 → NAS 桥。

---

## 1. 硬件准备

### 1.1 终端主控板（移动机器人）

| 组件 | 型号/规格 | 数量 | 备注 |
|---|---|---|---|
| 主控 | ESP32-S3-WROOM-1 N8R8 | 1 | 推荐立创实战派 ESP32-S3 开发板 |
| 麦克风 | 双 MEMS（INMP441） | 2 | 麦阵列，指向性拾音 |
| 喇叭 | 3W 8Ω + MAX98357A | 1 | I2S 功放 |
| 屏幕 | 1.28" GC9A01 圆屏 | 1 | 表情 UI |
| 灯带 | WS2812 ×8 | 1 | 呼吸灯反馈 |
| 电机 | N20 减速电机 + 编码器 | 2 | 差速驱动 |
| 电机驱动 | TB6612FNG | 1 | 双路电机驱动 |
| 万向轮 | 1.5" 万向轮 | 1 | 支撑 |
| 电池 | 18650 ×3 (11.1V) + 保护板 | 1 | 供电 |
| 避障 | HC-SR04 超声波 | 1 | 防撞 |
| IMU | MPU6050 | 1 | 姿态/航向 |
| 电源 | 5V/12V 稳压 | 2 | 主控+电机分离供电 |

### 1.2 智能节点（BLE 从机）

| 节点 | 核心 | 外设 |
|---|---|---|
| 灯节点 | ESP32-C3 SuperMini | MOSFET 调光 + WS2812 |
| 窗帘节点 | ESP32-C3 | 28BYJ-48 步进 + ULN2003 + 限位 |
| 插座节点 | ESP32-C3 | 继电器 + HLW8032 计量 |
| 传感器节点 | ESP32-C3 | SHT40 + SGP40 + BH1750 |

完整清单见 [hardware/BOM.csv](../hardware/BOM.csv)，PCB 工程见 [hardware/](../hardware/README.md)。

## 2. 终端固件

### 2.1 基础：跑通小智

```bash
# 基于 xiaozhi-esp32 官方工程二次开发
git clone https://github.com/78/xiaozhi-esp32
cd xiaozhi-esp32
# 将本仓库 firmware/terminal 下的 main/ 合并到工程 main/ 目录
# 配置 WiFi / 服务器地址（小智官方免费服务器：websocket://xyz.xiaozhi.me/ws）
idf.py set-target esp32s3
idf.py menuconfig        # 配置麦克风/喇叭/屏幕外设；启用蓝牙与电机PWM
idf.py build flash monitor
```

> 详细配置参考 xiaozhi-esp32 官方文档。本仓库提供的是 **MCP 工具集 + 运动控制 + WiFi/BLE 双通道 + 主动引擎** 的二次开发模块。

### 2.2 MCP 工具注册

`firmware/terminal/main/mcp_tools.c` 中注册以下工具（大模型可调用）：

| 工具名 | 参数 | 说明 |
|---|---|---|
| `smart_home_control` | device, cmd, value | 通用控制（WiFi联网/BLE 双通道路由）：灯/插座/窗帘/空调 |
| `robot_move` | action, speed, distance | 移动底盘：前进/后退/左转/右转/停止/召唤/巡航/跟随 |
| `query_sensor` | node_id, type | 查询 BLE 传感器节点 |
| `nas_query` | scope, query | NAS 知识中枢查询 |
| `scene_execute` | scene | 场景联动 |

### 2.3 分区表

`partitions.csv` 使用 4MB Flash 标准分区（含 assets 分区，支持 OTA 更新唤醒词/字体/背景）。

## 3. 节点固件

每个节点独立工程，统一 BLE GATT 协议（见 `firmware/node/README.md`）：

```bash
idf.py set-target esp32c3
idf.py build flash monitor
```

配置项：`NODE_TYPE`（light/curtain/plug/sensor）、`NODE_ID`、BLE 广播名（`XZ-<type>-<id>`）。

## 4. 控制中枢网关（可选，树莓派/NAS/PC）

```bash
cd gateway
pip install -r requirements.txt
python gateway.py --mqtt-broker 192.168.1.10 --config scenes.yaml
```

功能：
- MQTT Broker 桥接（EMQX/Mosquitto），WiFi 联网通道
- 设备管理（米家/Home Assistant 生态接入配置）
- 场景引擎（`scenes.py`）
- 设备状态聚合上报

## 5. NAS 知识中枢

```bash
cd nas_bridge
pip install -r requirements.txt
python bridge.py --nas-type synology --host 192.168.1.10 --user admin
python rag_server.py --index ./index --docs ./家庭文档
```

## 6. 看板

浏览器打开 `app/dashboard.html`，或部署到任意静态服务器。

## 7. 验证清单

- [ ] 小智唤醒 + 对话正常
- [ ] "小智过来" → 差速底盘移动到你身边
- [ ] "去卧室关灯" → 移动 + BLE 关灯
- [ ] 定时巡检 → 全屋巡航采集环境
- [ ] "打开客厅灯" → BLE 灯节点点亮
- [ ] "空调 26 度" → WiFi 联网控制空调（米家/HA）
- [ ] MQTT 桥接 Home Assistant / 米家生态设备
- [ ] "现在室温多少" → 传感器节点回读
- [ ] "明天上午日程" → NAS 日历播报
- [ ] 睡眠模式场景联动
- [ ] 断网时"打开客厅灯"本地 BLE 降级控制
- [ ] 主动提醒（温度异常/久坐/晨报）
