# 控制中枢网关

局域网控制中枢服务（Python），可运行在 **树莓派 / NAS(Docker) / 局域网PC** 上。

## 功能

- **MQTT Broker 桥接**：连接 EMQX/Mosquitto，转发小智终端与智能生态设备的控制/状态消息
- **红外码库管理**：`ir_codes/` 存储空调/电视等品牌的 NEC/RAW 码，支持"学习录入"
- **场景引擎**：`scenes.yaml` 声明场景动作组合，提供 REST API 触发
- **设备状态聚合**：收集各通道设备状态，统一上报（供看板/小智回读确认）

## 启动

```bash
pip install -r requirements.txt
python gateway.py --mqtt-broker 192.168.1.10 --config scenes.yaml
```

## REST API

| 方法 | 路径 | 说明 |
|---|---|---|
| GET | `/api/status` | 全部设备状态 |
| POST | `/api/device/{name}/cmd` | 发送设备指令 `{"cmd":"on","value":0}` |
| POST | `/api/scene/{name}` | 触发场景 |
| GET | `/api/ir/codes` | 列出红外码库 |
| POST | `/api/ir/learn` | 学习录入红外码 |

## 场景配置示例 (scenes.yaml)

```yaml
scenes:
  sleep:
    actions:
      - device: living_room_light
        cmd: set_brightness
        value: 5
      - device: living_room_curtain
        cmd: close
      - device: bedroom_ac
        cmd: set_temp
        value: 26
      - device: bedroom_plug
        cmd: off
    confirm: "睡眠模式已执行：灯光已调暗，窗帘已关闭，空调已设置。"
```

## 消息格式（与小智终端 MQTT 一致）

- 控制: `smart_home/{device}/cmd` `{"cmd":"set_brightness","value":30,"request_id":"..."}`
- 状态: `smart_home/{device}/status` `{"state":"on","value":30,"ts":1768500000}`
