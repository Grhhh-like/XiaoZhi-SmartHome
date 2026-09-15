# -*- coding: utf-8 -*-
"""
gen_pcb_preview.py - 生成 PCB 预览示意图
小智·智家 XiaoZhi SmartHome
使用 Pillow 绘制终端主控板 PCB 布局示意（无需 EDA 导出）
"""
import os

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    print("Pillow 未安装，跳过 PCB 预览生成")
    raise SystemExit(0)

OUT = os.path.join(os.path.dirname(__file__), "..", "assets", "pcb_preview.png")

W, H = 1000, 750
BOARD = (60, 60, 940, 690)
COLOR_BG = (15, 23, 42)
COLOR_BOARD = (20, 55, 40)      # PCB 基色（绿）
COLOR_TRACE = (220, 220, 190)   # 走线
COLOR_PAD = (200, 170, 90)      # 焊盘
COLOR_COMP = (230, 230, 230)    # 器件丝印


def load_font(size):
    for name in ("msyh.ttc", "simhei.ttf", "arial.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except Exception:
            continue
    return ImageFont.load_default()


def main():
    img = Image.new("RGB", (W, H), COLOR_BG)
    d = ImageDraw.Draw(img)
    font = load_font(22)
    font_s = load_font(16)

    # 板框
    d.rounded_rectangle(BOARD, radius=18, fill=COLOR_BOARD, outline=(80, 120, 90), width=4)

    # 走线（示意）
    import random
    random.seed(42)
    for _ in range(120):
        x1 = random.randint(BOARD[0]+30, BOARD[2]-30)
        y1 = random.randint(BOARD[1]+30, BOARD[3]-30)
        x2 = x1 + random.randint(-120, 120)
        y2 = y1 + random.randint(-80, 80)
        d.line([(x1, y1), (x2, y2)], fill=COLOR_TRACE, width=2)

    # 主控模组
    d.rounded_rectangle((360, 240, 640, 440), radius=8, outline=COLOR_COMP, width=3)
    d.text((500-70, 320), "ESP32-S3", fill=(255,255,255), font=font)
    d.text((500-90, 352), "主控模组", fill=(200,200,200), font=font_s)
    d.text((500-110, 382), "WiFi + BLE 双模", fill=(160,200,160), font=font_s)

    # 电机驱动 TB6612
    d.rounded_rectangle((690, 240, 840, 320), radius=6, outline=COLOR_COMP, width=2)
    d.text((705, 265), "TB6612 驱动", fill=(200,200,200), font=font_s)
    d.text((705, 290), "双N20电机", fill=(160,200,160), font=load_font(13))

    # 麦克风 x2
    for dx in (150, 300):
        d.rounded_rectangle((360-dx, 480, 400-dx, 520), radius=5, outline=COLOR_COMP, width=2)
        d.text((380-dx-22, 492), "MIC", fill=(200,200,200), font=font_s)

    # 功放/喇叭
    d.rounded_rectangle((690, 120, 820, 200), radius=6, outline=COLOR_COMP, width=2)
    d.text((720, 145), "功放", fill=(200,200,200), font=font_s)

    # 圆屏
    d.ellipse((120, 100, 320, 300), outline=COLOR_COMP, width=3)
    d.text((190, 185), "圆屏", fill=(200,200,200), font=font_s)

    # WiFi/BLE 天线区
    d.rounded_rectangle((120, 520, 300, 580), radius=5, outline=COLOR_COMP, width=2)
    d.text((135, 535), "WiFi/BLE 天线", fill=(200,200,200), font=font_s)
    d.text((135, 556), "2.4G", fill=(160,200,160), font=load_font(13))

    # 电机/编码器接口
    d.rounded_rectangle((690, 480, 840, 540), radius=5, outline=COLOR_COMP, width=2)
    d.text((705, 495), "电机+编码器接口", fill=(200,200,200), font=font_s)

    # 灯带接口
    d.rounded_rectangle((700, 560, 850, 620), radius=5, outline=COLOR_COMP, width=2)
    d.text((715, 575), "灯带接口", fill=(200,200,200), font=font_s)

    # USB-C
    d.rounded_rectangle((430, 580, 570, 640), radius=6, outline=COLOR_COMP, width=2)
    d.text((455, 600), "USB-C", fill=(200,200,200), font=font_s)

    # 标题
    d.text((500, 20), "小智·智家 移动机器人终端 PCB 布局示意", fill=(230,230,230),
           font=load_font(26), anchor="mm")
    d.text((500, 700), "100mm × 80mm 双层板 · 差速底盘(双N20+编码器) · 嘉立创EDA工程见 hardware/", fill=(150,150,150),
           font=font_s, anchor="mm")

    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    img.save(OUT, "PNG")
    print(f"generated: {OUT} ({W}x{H})")


if __name__ == "__main__":
    main()
