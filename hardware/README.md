# 硬件设计

「小智·智家」硬件分为 **终端主控板** 与 **智能节点板** 两部分，均提供嘉立创 EDA 工程（JSON）与 BOM。

## 目录

| 文件 | 说明 |
|---|---|
| `BOM.csv` | 完整物料清单（终端 + 节点） |
| `terminal_LCEDA.json` | 终端主控板嘉立创 EDA 工程（生成脚本见 scripts/） |
| `node_LCEDA.json` | 智能节点板嘉立创 EDA 工程 |

## 终端主控板（ESP32-S3）

- ESP32-S3-WROOM-1 N8R8 模组
- 双 MEMS 麦克风（INMP441）——语音交互
- I2S 功放 MAX98357A + 3W 喇叭
- 1.28" GC9A01 圆屏（表情 UI）
- WS2812 灯带 ×8（状态呼吸灯）
- 红外发射（38kHz，控制老家电）
- 433MHz 发射模块（FS1000A，无线插座）
- 5V→3.3V 电源

## 智能节点板（ESP32-C3）

统一底板 + 外设扩展：

- ESP32-C3 SuperMini 核心
- 灯节点：MOSFET 调光 + WS2812
- 窗帘节点：28BYJ-48 + ULN2003 + 限位
- 插座节点：继电器 + HLW8032
- 传感器节点：SHT40 + SGP40 + BH1750

## 使用

1. 在嘉立创 EDA 专业版导入 `terminal_LCEDA.json` / `node_LCEDA.json`
2. 按 BOM 采购物料
3. PCB 打样（嘉立创，约 5 元 5 片）
4. 焊接 + 烧录固件（见 firmware/）

> 工程 JSON 由 `scripts/gen_lceda_json.py` 生成，为原理图框架/起始工程，
> 实际布线需在 EDA 中完成（详见硬件 README 说明）。
