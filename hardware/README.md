# 硬件设计

「小智·智家」硬件分为 **移动机器人终端**（ESP32-S3 + 差速底盘）与 **智能节点板**（ESP32-C3）两部分，均提供嘉立创 EDA 工程（JSON）、BOM 与采购建议。

## 目录

| 文件 | 说明 |
|---|---|
| `BOM.csv` | 完整物料清单（终端 + 底盘 + 节点，约 300 元） |
| `terminal_LCEDA.json` | 终端主控板嘉立创 EDA 工程（含电机/编码器接口，生成脚本见 scripts/） |
| `node_LCEDA.json` | 智能节点板嘉立创 EDA 工程 |
| 3D 模型 | 见下方「3D 打印图纸」章节 |

## 是否需要自己画电路板？

| 模块 | 起步方案（推荐） | 进阶方案（毕设加分） |
|---|---|---|
| **终端主控** | 用现成开发板：立创实战派 ESP32-S3 开发板（已带双麦/喇叭/屏幕接口），零焊接 | 用 `terminal_LCEDA.json` 在嘉立创 EDA 画自定义底板（含电机驱动/电源管理），打样免费 |
| **移动底盘** | 买现成差速底盘套件（约 40-60 元，含电机/轮子/电池仓） | 用下方 3D 模型 3D 打印底盘，配 N20 电机自组 |
| **智能节点 ×4** | ESP32-C3 SuperMini + 洞洞板/面包板搭建（最快验证） | 画统一节点底板（`node_LCEDA.json`），4 节点共用，嘉立创 5 元打样 |

**结论**：功能验证阶段 **不需要画板**（开发板 + 模块 + 3D 打印即可跑通全系统）；建议进入毕设/量产阶段再画终端底板与节点板——嘉立创打样补贴正好覆盖。

## 移动机器人终端（ESP32-S3）

- ESP32-S3-WROOM-1 N8R8 模组（WiFi + BLE 双模，参考米家模式）
- 双 MEMS 麦克风（INMP441）——语音交互
- I2S 功放 MAX98357A + 3W 喇叭
- 1.28" GC9A01 圆屏（表情 UI）+ WS2812 灯带（状态呼吸灯）
- **差速底盘**：双 N20 减速电机（30:1）+ 霍尔编码器 + TB6612 驱动 + 万向轮
- 18650×3 电池（11.1V）+ DC-DC 5V 双电源
- 超声波避障（HC-SR04）+ IMU（MPU6050）

## 智能节点板（ESP32-C3）

统一底板 + 外设扩展（BLE 从机，广播名 XZ-<type>-<id>）：

- ESP32-C3 SuperMini 核心
- 灯节点：MOSFET 调光 + WS2812
- 窗帘节点：28BYJ-48 + ULN2003 + 限位
- 插座节点：继电器 + HLW8032
- 传感器节点：SHT40 + SGP40 + BH1750

## 3D 打印图纸（开源参考）

适合本项目 35cm 级差速机器人的开源模型：

| 模型 | 说明 | 来源 |
|---|---|---|
| **The 50$ ESP32 Rover** | 全 3D 打印 ESP32 差速底盘，可换电机外壳，TPU 无气轮，全套件 ~50 美元，最贴近本项目 | [MakerWorld](https://makerworld.com/en/models/2621918-the-50-esp32-rover) |
| **ESP32 Robot w/ Odometry** | 带编码器里程计 + 航点导航的 ESP32 机器人，STL 可下载，PLA 0.28mm 打印参数 | [Bench Robotics](https://benchrobotics.com/robotics/esp32-robot-with-odometry-and-waypoint-navigation/) |
| **ESP-Bot** | 小型全 3D 打印底盘 + 车轮，无需支撑，小体积打印机即可 | [gaysimon.github.io](https://gaysimon.github.io/robot/ESPbot_en.html) |
| **Papaya Pathfinder** | 开源 ESP32 rover，Rocker-Bogie 悬挂，多种尺寸变体 | [Hackaday.io](https://hackaday.io/project/204911-papaya-pathfinder-open-source-esp32-rover/details) |
| **DART Robot** | 底盘 + 上层电子仓分离设计，PLA/PETG 打印包 | [DigInto](https://diginto.tech/robot-builds/dart) |

> 本项目底盘为自研差速方案（N20 电机 + 编码器），可直接参考上述模型的结构布局；
> 若需与「圆屏表情 + 双麦 + 喇叭」契合的外壳，推荐基于 `The 50$ ESP32 Rover` 上层电子仓改制。

## 采购渠道（国内）

| 渠道 | 用途 |
|---|---|
| 立创商城（szlcsc.com） | 元器件、ESP32 模组、传感器、嘉立创打样 |
| 淘宝/1688 | N20 电机带编码器、差速底盘套件、18650 电池 |
| 嘉立创 EDA 打样 | PCB 5 元 5 片（星火计划可申请耗材资助） |
| 立创开源硬件平台 | 同款/可复用项目参考与器件库 |

## 使用

1. 按「是否需要自己画电路板」选择起步/进阶方案
2. 在嘉立创 EDA 专业版导入 `terminal_LCEDA.json` / `node_LCEDA.json`
3. 按 BOM 采购物料（参考上方渠道）
4. 3D 打印或购买底盘，组装 + 焊接 + 烧录固件（见 firmware/）

> 工程 JSON 由 `scripts/gen_lceda_json.py` 生成，为原理图框架/起始工程，
> 实际布线需在 EDA 中完成（详见硬件 README 说明）。
