/*
 * ble_gateway.h - 蓝牙 BLE 网关（终端作为 BLE Central）
 * 小智·智家 XiaoZhi SmartHome
 *
 * 参考米家智能家居模式：自制智能节点（ESP32-C3）以 BLE GATT
 * 从机形式组网，终端作为 BLE 主机（Central）扫描、连接并控制。
 * 统一广播名：XZ-<type>-<id>，例如 XZ-light-living、XZ-sensor-kitchen。
 */
#ifndef BLE_GATEWAY_H
#define BLE_GATEWAY_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* BLE GATT 服务/特征值 UUID（统一命名空间） */
#define BLE_GW_SERVICE_UUID        "0000XZ00-0000-1000-8000-00805F9B34FB"
#define BLE_GW_CHAR_CTRL_UUID      "0000XZ01-0000-1000-8000-00805F9B34FB" /* Write: 控制 */
#define BLE_GW_CHAR_STATE_UUID     "0000XZ02-0000-1000-8000-00805F9B34FB" /* Notify: 状态上报 */

/* 控制命令帧（JSON 简化为紧凑二进制，按需扩展） */
typedef struct {
    char    device[24];   /* 节点ID: living_room_light */
    char    cmd[16];      /* on / off / set_brightness / set_temp / open / close / stop */
    int32_t value;        /* 数值参数 */
} ble_ctrl_frame_t;

/* ---------- API ---------- */

/* 初始化 BLE 网关（启动扫描，自动重连已配对节点） */
void ble_gateway_init(void);

/* 控制灯节点（亮度 0-100） */
int ble_light_ctrl(const char *device, const char *cmd, int32_t value);

/* 控制窗帘节点（open/close/stop/百分比） */
int ble_curtain_ctrl(const char *device, const char *cmd, int32_t value);

/* 控制插座节点（on/off） */
int ble_plug_ctrl(const char *device, const char *cmd);

/* 读取传感器节点数据（type: temp/humi/air/light） */
int ble_sensor_read(const char *node_id, const char *type, float *out_value);

/* 周期任务：扫描/重连/状态订阅（由 house_task 调用） */
void ble_gateway_tick(void);

#ifdef __cplusplus
}
#endif

#endif /* BLE_GATEWAY_H */
