# -*- coding: utf-8 -*-
"""
gen_arch_svg.py - 生成「小智·智家」体系架构图 SVG
小智·智家 XiaoZhi SmartHome
"""
import os

OUT = os.path.join(os.path.dirname(__file__), "..", "assets", "architecture.svg")

SVG = '''<svg xmlns="http://www.w3.org/2000/svg" width="1200" height="900" viewBox="0 0 1200 900">
  <defs>
    <linearGradient id="bg" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0%" stop-color="#0f172a"/>
      <stop offset="100%" stop-color="#1e293b"/>
    </linearGradient>
    <linearGradient id="app" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#22d3ee"/>
      <stop offset="100%" stop-color="#3b82f6"/>
    </linearGradient>
    <linearGradient id="cloud" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#a855f7"/>
      <stop offset="100%" stop-color="#6366f1"/>
    </linearGradient>
    <linearGradient id="ai" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#f59e0b"/>
      <stop offset="100%" stop-color="#ef4444"/>
    </linearGradient>
    <linearGradient id="gw" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#10b981"/>
      <stop offset="100%" stop-color="#14b8a6"/>
    </linearGradient>
    <linearGradient id="exec" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#f43f5e"/>
      <stop offset="100%" stop-color="#f97316"/>
    </linearGradient>
    <filter id="shadow">
      <feDropShadow dx="0" dy="3" stdDeviation="6" flood-opacity="0.35"/>
    </filter>
  </defs>

  <rect width="1200" height="900" fill="url(#bg)"/>

  <!-- 标题 -->
  <text x="600" y="52" text-anchor="middle" font-size="30" font-weight="bold" fill="#e2e8f0" font-family="Microsoft YaHei">小智·智家 XiaoZhi SmartHome — AI 语音万物互联家庭管家体系</text>

  <!-- ============ 应用层 ============ -->
  <rect x="60" y="80" width="1080" height="110" rx="14" fill="url(#app)" opacity="0.92" filter="url(#shadow)"/>
  <text x="90" y="112" font-size="18" font-weight="bold" fill="#fff" font-family="Microsoft YaHei">应用层</text>
  <g font-family="Microsoft YaHei" font-size="15" fill="#fff">
    <rect x="90" y="125" width="150" height="46" rx="8" fill="#ffffff22"/>
    <text x="165" y="153" text-anchor="middle">手机 APP</text>
    <rect x="260" y="125" width="150" height="46" rx="8" fill="#ffffff22"/>
    <text x="335" y="153" text-anchor="middle">网页看板</text>
    <rect x="430" y="125" width="150" height="46" rx="8" fill="#ffffff22"/>
    <text x="505" y="153" text-anchor="middle">场景模式</text>
    <rect x="600" y="125" width="150" height="46" rx="8" fill="#ffffff22"/>
    <text x="675" y="153" text-anchor="middle">状态监控</text>
    <rect x="770" y="125" width="150" height="46" rx="8" fill="#ffffff22"/>
    <text x="845" y="153" text-anchor="middle">告警推送</text>
  </g>

  <!-- ============ 云平台层 ============ -->
  <rect x="60" y="210" width="1080" height="110" rx="14" fill="url(#cloud)" opacity="0.92" filter="url(#shadow)"/>
  <text x="90" y="242" font-size="18" font-weight="bold" fill="#fff" font-family="Microsoft YaHei">云平台</text>
  <g font-family="Microsoft YaHei" font-size="15" fill="#fff">
    <rect x="90" y="255" width="200" height="46" rx="8" fill="#ffffff22"/>
    <text x="190" y="283" text-anchor="middle">小智 AI 服务器(大模型)</text>
    <rect x="310" y="255" width="180" height="46" rx="8" fill="#ffffff22"/>
    <text x="400" y="283" text-anchor="middle">MQTT Broker</text>
    <rect x="510" y="255" width="180" height="46" rx="8" fill="#ffffff22"/>
    <text x="600" y="283" text-anchor="middle">NAS 私有云</text>
    <rect x="710" y="255" width="200" height="46" rx="8" fill="#ffffff22"/>
    <text x="810" y="283" text-anchor="middle">Home Assistant 生态</text>
  </g>

  <!-- ============ AI 决策层 ============ -->
  <rect x="60" y="340" width="1080" height="120" rx="14" fill="url(#ai)" opacity="0.92" filter="url(#shadow)"/>
  <text x="90" y="372" font-size="18" font-weight="bold" fill="#fff" font-family="Microsoft YaHei">AI 决策层</text>
  <g font-family="Microsoft YaHei" font-size="14" fill="#fff">
    <rect x="90" y="385" width="150" height="54" rx="8" fill="#ffffff22"/>
    <text x="165" y="411" text-anchor="middle">流式 ASR</text>
    <text x="165" y="430" text-anchor="middle" font-size="12">语音识别</text>
    <rect x="260" y="385" width="150" height="54" rx="8" fill="#ffffff22"/>
    <text x="335" y="411" text-anchor="middle">大模型 LLM</text>
    <text x="335" y="430" text-anchor="middle" font-size="12">豆包/Qwen/DeepSeek</text>
    <rect x="430" y="385" width="150" height="54" rx="8" fill="#ffffff22"/>
    <text x="505" y="411" text-anchor="middle">意图理解</text>
    <text x="505" y="430" text-anchor="middle" font-size="12">工具调用</text>
    <rect x="600" y="385" width="150" height="54" rx="8" fill="#ffffff22"/>
    <text x="675" y="411" text-anchor="middle">RAG 检索</text>
    <text x="675" y="430" text-anchor="middle" font-size="12">NAS 文档问答</text>
    <rect x="770" y="385" width="150" height="54" rx="8" fill="#ffffff22"/>
    <text x="845" y="411" text-anchor="middle">主动引擎</text>
    <text x="845" y="430" text-anchor="middle" font-size="12">习惯学习/提醒</text>
    <rect x="940" y="385" width="170" height="54" rx="8" fill="#ffffff22"/>
    <text x="1025" y="411" text-anchor="middle">TTS 播报</text>
    <text x="1025" y="430" text-anchor="middle" font-size="12">状态回读确认</text>
  </g>

  <!-- ============ 控制中枢层 ============ -->
  <rect x="60" y="480" width="1080" height="150" rx="14" fill="url(#gw)" opacity="0.92" filter="url(#shadow)"/>
  <text x="90" y="512" font-size="18" font-weight="bold" fill="#fff" font-family="Microsoft YaHei">控制中枢（小智终端 ESP32-S3）</text>
  <g font-family="Microsoft YaHei" font-size="14" fill="#fff">
    <rect x="90" y="525" width="190" height="84" rx="8" fill="#ffffff22"/>
    <text x="185" y="552" text-anchor="middle">MCP 工具集</text>
    <text x="185" y="572" text-anchor="middle" font-size="12">开关灯/调空调/拉窗帘/查询</text>
    <text x="185" y="592" text-anchor="middle" font-size="12">6 个工具 schema</text>
    <rect x="300" y="525" width="160" height="84" rx="8" fill="#ffffff22"/>
    <text x="380" y="552" text-anchor="middle">红外发射</text>
    <text x="380" y="572" text-anchor="middle" font-size="12">38kHz NEC/RAW</text>
    <text x="380" y="592" text-anchor="middle" font-size="12">空调/电视/风扇</text>
    <rect x="480" y="525" width="160" height="84" rx="8" fill="#ffffff22"/>
    <text x="560" y="552" text-anchor="middle">433MHz 射频</text>
    <text x="560" y="572" text-anchor="middle" font-size="12">无线插座/灯</text>
    <text x="560" y="592" text-anchor="middle" font-size="12">FS1000A</text>
    <rect x="660" y="525" width="160" height="84" rx="8" fill="#ffffff22"/>
    <text x="740" y="552" text-anchor="middle">MQTT 桥</text>
    <text x="740" y="572" text-anchor="middle" font-size="12">智能生态接入</text>
    <text x="740" y="592" text-anchor="middle" font-size="12">状态订阅回读</text>
    <rect x="840" y="525" width="160" height="84" rx="8" fill="#ffffff22"/>
    <text x="920" y="552" text-anchor="middle">ESP-NOW 组网</text>
    <text x="920" y="572" text-anchor="middle" font-size="12">自制节点控制</text>
    <text x="920" y="592" text-anchor="middle" font-size="12">无路由器直连</text>
    <rect x="1020" y="525" width="100" height="84" rx="8" fill="#ffffff22"/>
    <text x="1070" y="552" text-anchor="middle">离线降级</text>
    <text x="1070" y="572" text-anchor="middle" font-size="12">断网命令词</text>
    <text x="1070" y="592" text-anchor="middle" font-size="12">本地兜底</text>
  </g>

  <!-- ============ 执行层 ============ -->
  <rect x="60" y="650" width="1080" height="190" rx="14" fill="url(#exec)" opacity="0.92" filter="url(#shadow)"/>
  <text x="90" y="682" font-size="18" font-weight="bold" fill="#fff" font-family="Microsoft YaHei">执行层（万物互联）</text>
  <g font-family="Microsoft YaHei" font-size="14" fill="#fff">
    <rect x="90" y="695" width="170" height="120" rx="8" fill="#ffffff22"/>
    <text x="175" y="722" text-anchor="middle">智能灯节点</text>
    <text x="175" y="742" text-anchor="middle" font-size="12">ESP32-C3</text>
    <text x="175" y="762" text-anchor="middle" font-size="12">PWM 调光 0-100%</text>
    <text x="175" y="782" text-anchor="middle" font-size="12">RGB 灯带</text>
    <rect x="280" y="695" width="170" height="120" rx="8" fill="#ffffff22"/>
    <text x="365" y="722" text-anchor="middle">窗帘电机节点</text>
    <text x="365" y="742" text-anchor="middle" font-size="12">28BYJ-48 步进</text>
    <text x="365" y="762" text-anchor="middle" font-size="12">开/关/百分比</text>
    <text x="365" y="782" text-anchor="middle" font-size="12">限位校准</text>
    <rect x="470" y="695" width="170" height="120" rx="8" fill="#ffffff22"/>
    <text x="555" y="722" text-anchor="middle">智能插座节点</text>
    <text x="555" y="742" text-anchor="middle" font-size="12">继电器开关</text>
    <text x="555" y="762" text-anchor="middle" font-size="12">HLW8032 计量</text>
    <text x="555" y="782" text-anchor="middle" font-size="12">能耗统计</text>
    <rect x="660" y="695" width="170" height="120" rx="8" fill="#ffffff22"/>
    <text x="745" y="722" text-anchor="middle">环境传感器节点</text>
    <text x="745" y="742" text-anchor="middle" font-size="12">SHT40/SGP40/BH1750</text>
    <text x="745" y="762" text-anchor="middle" font-size="12">周期上报 30s</text>
    <text x="745" y="782" text-anchor="middle" font-size="12">低功耗待机</text>
    <rect x="850" y="695" width="170" height="120" rx="8" fill="#ffffff22"/>
    <text x="935" y="722" text-anchor="middle">摄像头联动</text>
    <text x="935" y="742" text-anchor="middle" font-size="12">"谁来了"识别</text>
    <text x="935" y="762" text-anchor="middle" font-size="12">手势控制</text>
    <text x="935" y="782" text-anchor="middle" font-size="12">安防布防</text>
    <rect x="1040" y="695" width="80" height="120" rx="8" fill="#ffffff22"/>
    <text x="1080" y="750" text-anchor="middle" font-size="12">红外</text>
    <text x="1080" y="770" text-anchor="middle" font-size="12">空调</text>
    <text x="1080" y="790" text-anchor="middle" font-size="12">电视</text>
  </g>

  <!-- 层间连接箭头 -->
  <g stroke="#475569" stroke-width="2" opacity="0.8">
    <line x1="600" y1="190" x2="600" y2="210"/>
    <line x1="600" y1="320" x2="600" y2="340"/>
    <line x1="600" y1="460" x2="600" y2="480"/>
    <line x1="600" y1="630" x2="600" y2="650"/>
  </g>

  <!-- 标注 -->
  <text x="600" y="868" text-anchor="middle" font-size="13" fill="#94a3b8" font-family="Microsoft YaHei">感知 → 决策 → 执行 → 确认 全链路闭环 · 主动化 / 互动性 / 智能化 / 万物互联</text>
</svg>
'''

def main():
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        f.write(SVG)
    print(f"generated: {OUT}")


if __name__ == "__main__":
    main()
