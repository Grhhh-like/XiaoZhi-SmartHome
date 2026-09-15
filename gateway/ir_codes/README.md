# 红外码库

存储空调/电视/风扇等老家电的红外控制码，供小智终端/网关查询发射。

## 格式

每个品牌一个 JSON 文件，示例 `gree_ac.json`：

```json
{
  "brand": "gree",
  "device": "ac",
  "protocol": "nec",
  "codes": {
    "power_on":  {"addr": 0x00, "cmd": 0x08},
    "power_off": {"addr": 0x00, "cmd": 0x08},
    "mode_cool": {"addr": 0x00, "cmd": 0x09},
    "mode_heat": {"addr": 0x00, "cmd": 0x0A},
    "temp_16":   {"addr": 0x00, "cmd": 0x10},
    "temp_26":   {"addr": 0x00, "cmd": 0x1A},
    "fan_auto":  {"addr": 0x00, "cmd": 0x20}
  }
}
```

也支持 RAW 码格式：

```json
{
  "brand": "sony",
  "device": "tv",
  "protocol": "raw",
  "codes": {
    "power_on": {"raw": [2400, 600, 1200, 600, 600, 600, 1200]}
  }
}
```

## 学习录入

1. 将红外接收头接到终端（或使用网关串口学习器）
2. 调用 `POST /api/ir/learn` 并按下遥控器按键
3. 网关解析 NEC/RAW 波形并写入码库 JSON
4. 小智终端下次即可直接发射

> 注意：不同品牌/型号空调码差异较大，**实际使用请通过学习录入获取真实码**。
> 仓库内置码为格式示例，非保证可用的真实空调码。
