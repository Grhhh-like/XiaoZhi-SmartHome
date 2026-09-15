# -*- coding: utf-8 -*-
"""
gen_lceda_json.py - 生成嘉立创EDA工程起始文件
小智·智家 XiaoZhi SmartHome

生成可导入嘉立创EDA专业版的 JSON 起始工程（终端主控板 / 智能节点板）。
说明：此文件为原理图框架/器件清单起始工程，布线布局需在 EDA 中完成。
"""
import json
import os

OUT_DIR = os.path.join(os.path.dirname(__file__), "..", "hardware")


def build_terminal_project():
    """机器人终端主控板工程（含移动底盘接口）"""
    return {
        "name": "XiaoZhi-SmartHome-Terminal",
        "version": "2.0.0",
        "description": "小智·智家 移动机器人终端（ESP32-S3 + 双麦 + 喇叭 + 圆屏 + 差速底盘 + WiFi/BLE）",
        "board": {
            "width_mm": 100.0,
            "height_mm": 80.0,
            "layers": 2,
        },
        "components": [
            {"ref": "U1",  "value": "ESP32-S3-WROOM-1", "package": "SMD-MODULE-38", "designator": "主控模组(WiFi+BLE)"},
            {"ref": "U2",  "value": "INMP441", "package": "LGA-4", "designator": "麦克风1"},
            {"ref": "U3",  "value": "INMP441", "package": "LGA-4", "designator": "麦克风2"},
            {"ref": "U4",  "value": "MAX98357A", "package": "TSSOP-16", "designator": "I2S功放"},
            {"ref": "U5",  "value": "GC9A01", "package": "COB-1.28", "designator": "圆屏"},
            {"ref": "U6",  "value": "WS2812", "package": "SMD-5050", "designator": "RGB灯带"},
            {"ref": "U7",  "value": "TB6612FNG", "package": "SSOP-24", "designator": "电机驱动"},
            {"ref": "U8",  "value": "MPU6050", "package": "QFN-24", "designator": "IMU"},
            {"ref": "U9",  "value": "HC-SR04", "package": "THT-4", "designator": "超声波避障"},
            {"ref": "M1",  "value": "N20+编码器", "package": "MOTOR-N20", "designator": "左轮电机"},
            {"ref": "M2",  "value": "N20+编码器", "package": "MOTOR-N20", "designator": "右轮电机"},
            {"ref": "BT1", "value": "18650×3", "package": "BATTERY-3S", "designator": "锂电池组"},
            {"ref": "U10", "value": "MP1584-5V", "package": "SOT-23-5", "designator": "DC-DC 5V"},
            {"ref": "U11", "value": "AMS1117-3.3", "package": "SOT-223", "designator": "LDO"},
            {"ref": "ANT1", "value": "2.4G-Antenna", "package": "PCB-IPEX", "designator": "WiFi/BLE天线"},
            {"ref": "J1",  "value": "USB-C", "package": "USB-C-16P", "designator": "供电/烧录"},
            {"ref": "J2",  "value": "PH2.0-4P", "package": "PH2.0", "designator": "喇叭接口"},
            {"ref": "J3",  "value": "PH2.0-4P", "package": "PH2.0", "designator": "灯带接口"},
            {"ref": "J4",  "value": "XH2.54-6P", "package": "XH2.54", "designator": "左电机+编码器"},
            {"ref": "J5",  "value": "XH2.54-6P", "package": "XH2.54", "designator": "右电机+编码器"},
        ],
        "nets": [
            "VCC_5V", "VCC_3V3", "GND", "I2S_SCK", "I2S_WS", "I2S_SD_OUT",
            "ANT_2G4", "SPI_CLK", "SPI_MOSI", "SPI_CS", "GPIO_DIMMER",
            "PWM_A", "PWM_B", "AIN1", "AIN2", "BIN1", "BIN2",
            "ENC_L_A", "ENC_L_B", "ENC_R_A", "ENC_R_B", "I2C_SDA", "I2C_SCL", "ECHO", "TRIG",
        ],
        "notes": [
            "此为起始工程，原理图器件清单与网络已建立，连线/布线/DRC 请在嘉立创EDA专业版中完成",
            "主控 5V 与电机 12V 分离供电，电机电源走粗线并加去耦电容",
            "WiFi/BLE 天线走线注意阻抗匹配（2.4G），天线区下方铺地挖空",
            "编码器信号加 10kΩ 上拉与 RC 滤波",
        ],
    }


def build_node_project():
    """智能节点板工程（通用底板）"""
    return {
        "name": "XiaoZhi-SmartHome-Node",
        "version": "1.0.0",
        "description": "小智·智家 智能节点板（ESP32-C3 通用底板 + 外设扩展）",
        "board": {
            "width_mm": 40.0,
            "height_mm": 40.0,
            "layers": 2,
        },
        "components": [
            {"ref": "U1", "value": "ESP32-C3-SuperMini", "package": "SMD-MODULE-26", "designator": "节点核心"},
            {"ref": "J1", "value": "USB-C", "package": "USB-C-16P", "designator": "供电/烧录"},
            {"ref": "J2", "value": "PH2.0-4P", "package": "PH2.0", "designator": "外设扩展"},
            {"ref": "U2", "value": "AMS1117-3.3", "package": "SOT-223", "designator": "LDO"},
            {"ref": "J3", "value": "XH2.54-4P", "package": "XH2.54", "designator": "传感器接口"},
        ],
        "nets": ["VCC_3V3", "GND", "GPIO3", "GPIO4", "GPIO5", "GPIO6"],
        "notes": [
            "通用底板，外设（灯/窗帘/插座/传感器）通过 J2/J3 扩展",
            "具体外设电路参考 firmware/node/ 各节点定义",
        ],
    }


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    terminal = os.path.join(OUT_DIR, "terminal_LCEDA.json")
    node = os.path.join(OUT_DIR, "node_LCEDA.json")

    with open(terminal, "w", encoding="utf-8") as f:
        json.dump(build_terminal_project(), f, ensure_ascii=False, indent=2)
    with open(node, "w", encoding="utf-8") as f:
        json.dump(build_node_project(), f, ensure_ascii=False, indent=2)

    print(f"generated: {terminal}")
    print(f"generated: {node}")


if __name__ == "__main__":
    main()
