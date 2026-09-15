# 自制智能节点（ESP-NOW 从机）

基于 ESP32-C3 SuperMini 的低成本智能节点，通过 ESP-NOW 与「小智终端」组网（无需路由器），
支持低功耗待机。四种节点类型：

| 目录 | 节点 | 外设 | 功能 |
|---|---|---|---|
| `light_node` | 智能灯 | MOSFET 调光 + WS2812 灯带 | 亮度 0-100% / RGB |
| `curtain_node` | 窗帘电机 | 28BYJ-48 步进电机 + ULN2003 + 限位开关 | 开/关/百分比 |
| `plug_node` | 智能插座 | 继电器 + HLW8032 计量芯片 | 开关 + 功率计量 |
| `sensor_node` | 环境传感器 | SHT40 + SGP40 + BH1750 | 温湿度/空气质量/光照 |

## 编译（每个节点独立工程）

```bash
idf.py set-target esp32c3
idf.py menuconfig   # 配置 NODE_TYPE / NODE_ID / 主节点MAC
idf.py build flash monitor
```

## ESP-NOW 协议（与终端一致）

```
[0] 帧类型(0x01控制/0x02状态/0x03传感器/0x04发现)
[1] 节点类型(0x01灯/0x02窗帘/0x03插座/0x04传感器)
[2-5] 节点ID(uint32, 设备名哈希)
[6] 命令(0x01开/0x02关/0x03设值/0x10查询/0xF0应答)
[7-...] 参数(变长)
[尾] CRC8
```

## 低功耗设计

- 主循环采用事件驱动 + light sleep：待机电流可降至 ~50µA
- 传感器节点按需唤醒采样上报，默认上报周期 30s
- 控制节点（灯/窗帘/插座）保持 ESP-NOW 监听，功耗 ~10mA 级别

## 节点配置（Kconfig）

| 配置项 | 默认 | 说明 |
|---|---|---|
| `NODE_TYPE` | sensor | light/curtain/plug/sensor |
| `NODE_ID` | 0x00000001 | 节点ID（终端设备名哈希需一致） |
| `MASTER_MAC` | FF:FF:FF:FF:FF:FF | 主节点 MAC（广播可 FF） |
| `SENSOR_INTERVAL_S` | 30 | 传感器上报周期 |
