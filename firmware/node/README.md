# 自制智能节点（BLE 从机）

基于 ESP32-C3 SuperMini 的低成本智能节点，通过 **BLE（蓝牙低功耗）** 与「小智终端」组网，
参考米家智能家居模式：节点为 BLE GATT Server（外设），终端为 BLE Central（主机）。
四种节点类型：

| 目录 | 节点 | 外设 | 功能 |
|---|---|---|---|
| `light_node` | 智能灯 | MOSFET 调光 + WS2812 灯带 | 亮度 0-100% / RGB |
| `curtain_node` | 窗帘电机 | 28BYJ-48 步进电机 + ULN2003 + 限位开关 | 开/关/百分比 |
| `plug_node` | 智能插座 | 继电器 + HLW8032 计量芯片 | 开关 + 功率计量 |
| `sensor_node` | 环境传感器 | SHT40 + SGP40 + BH1750 | 温湿度/空气质量/光照 |

## 编译（每个节点独立工程）

```bash
idf.py set-target esp32c3
idf.py menuconfig   # 启用蓝牙：Component config → Bluetooth → NimBLE
idf.py build flash monitor
```

## BLE 协议（与终端 ble_gateway 一致）

- 广播名：`XZ-<type>-<id>`（如 `XZ-01-00000001`）
- 服务 UUID：`0000XZ00-0000-1000-8000-00805F9B34FB`
- 控制特征值（Write）：接收 `ble_node_ctrl_t`（device/cmd/value）
- 状态特征值（Notify）：上报状态与传感器数据

详见 [ble_node/README.md](ble_node/README.md)。

## 低功耗设计

- 主循环采用事件驱动 + light sleep：待机电流可降至 ~50µA
- 传感器节点按需唤醒采样上报，默认上报周期 30s
- 控制节点（灯/窗帘/插座）保持 BLE 广播监听，功耗 ~10mA 级别

## 节点配置（Kconfig）

| 配置项 | 默认 | 说明 |
|---|---|---|
| `NODE_TYPE` | sensor | light/curtain/plug/sensor |
| `NODE_ID` | 0x00000001 | 节点ID（终端设备名需一致） |
| `SENSOR_INTERVAL_S` | 30 | 传感器上报周期 |
