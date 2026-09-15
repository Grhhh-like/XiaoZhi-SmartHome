/*
 * mqtt_bridge.h - MQTT 桥接模块
 * 小智·智家 XiaoZhi SmartHome
 * 桥接 Home Assistant / 巴法云 / 自建 Broker，实现智能生态设备控制与状态订阅
 */
#ifndef MQTT_BRIDGE_H
#define MQTT_BRIDGE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 初始化 MQTT 客户端并连接 Broker */
void mqtt_bridge_init(void);

/* 发布设备控制指令
 * 主题: smart_home/{device}/cmd
 * 载荷: {"cmd":"set_brightness","value":30,"request_id":"..."}
 */
void mqtt_publish_device(const char *device, const char *cmd, int32_t value);

/* 订阅设备状态主题，状态回调由调用方注册 */
void mqtt_subscribe_status(const char *device);

/* 回读最近一次设备状态（成功返回 0） */
int mqtt_get_device_status(const char *device, char *buf, int buf_len);

/* 断言：Broker 是否已连接 */
bool mqtt_is_connected(void);

#ifdef __cplusplus
}
#endif

#endif /* MQTT_BRIDGE_H */
