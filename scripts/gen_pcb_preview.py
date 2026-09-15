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
    d.rounded_rectangle((360, 260, 640, 460), radius=8, outline=COLOR_COMP, width=3)
    d.text((500-70, 340), "ESP32-S3", fill=(255,255,255), font=font)
    d.text((500-90, 372), "主控模组", fill=(200,200,200), font=font_s)

    # 麦克风 x2
    for dx in (150, 300):
        d.rounded_rectangle((360-dx, 500, 400-dx, 540), radius=5, outline=COLOR_COMP, width=2)
        d.text((380-dx-22, 512), "MIC", fill=(200,200,200), font=font_s)

    # 功放/喇叭
    d.rounded_rectangle((690, 150, 820, 230), radius=6, outline=COLOR_COMP, width=2)
    d.text((720, 175), "功放", fill=(200,200,200), font=font_s)

    # 圆屏
    d.ellipse((120, 120, 320, 320), outline=COLOR_COMP, width=3)
    d.text((190, 205), "圆屏", fill=(200,200,200), font=font_s)

    # 红外 + 433
    d.rounded_rectangle((120, 560, 240, 620), radius=5, outline=COLOR_COMP, width=2)
    d.text((135, 575), "红外", fill=(200,200,200), font=font_s)
    d.rounded_rectangle((260, 560, 380, 620), radius=5, outline=COLOR_COMP, width=2)
    d.text((275, 575), "433MHz", fill=(200,200,200), font=font_s)

    # 灯带接口
    d.rounded_rectangle((700, 500, 850, 560), radius=5, outline=COLOR_COMP, width=2)
    d.text((715, 515), "灯带接口", fill=(200,200,200), font=font_s)

    # USB-C
    d.rounded_rectangle((430, 600, 570, 660), radius=6, outline=COLOR_COMP, width=2)
    d.text((455, 620), "USB-C", fill=(200,200,200), font=font_s)

    # 标题
    d.text((500, 20), "小智·智家 终端主控板 PCB 布局示意", fill=(230,230,230),
           font=load_font(26), anchor="mm")
    d.text((500, 715), "80mm × 60mm 双层板 · 嘉立创EDA工程见 hardware/", fill=(150,150,150),
           font=font_s, anchor="mm")

    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    img.save(OUT, "PNG")
    print(f"generated: {OUT} ({W}x{H})")


if __name__ == "__main__":
    main()
