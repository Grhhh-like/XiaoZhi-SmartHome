/*
 * ble_node.h - 智能节点公共模块（BLE 从机 / GATT Server）
 * 小智·智家 XiaoZhi SmartHome
 *
 * 参考米家智能家居模式：节点以 BLE 从机形式组网，广播名 XZ-<type>-<id>，
 * 终端（BLE Central）扫描连接后通过控制特征值下发指令，
 * 节点通过状态特征值（Notify）上报状态/传感器数据。
 */
#ifndef BLE_NODE_H
#define BLE_NODE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 节点类型（与终端一致） */
#define NODE_LIGHT    0x01
#define NODE_CURTAIN  0x02
#define NODE_PLUG     0x03
#define NODE_SENSOR   0x04

/* 命令 */
#define CMD_ON         0x01
#define CMD_OFF        0x02
#define CMD_SET_VALUE  0x03
#define CMD_QUERY      0x10
#define CMD_ACK        0xF0

/* BLE GATT UUID（与终端 ble_gateway.h 保持一致） */
#define BLE_NODE_SERVICE_UUID  "0000XZ00-0000-1000-8000-00805F9B34FB"
#define BLE_NODE_CHAR_CTRL     "0000XZ01-0000-1000-8000-00805F9B34FB" /* Write */
#define BLE_NODE_CHAR_STATE    "0000XZ02-0000-1000-8000-00805F9B34FB" /* Notify */

/* 控制帧（与终端 ble_ctrl_frame_t 对齐） */
#pragma pack(push, 1)
typedef struct {
    char    device[24];   /* 节点ID */
    char    cmd[16];      /* on/off/set_brightness/open/close/stop/... */
    int32_t value;        /* 数值参数 */
} ble_node_ctrl_t;
#pragma pack(pop)

/* 节点回调：收到控制指令时触发（由各节点实现） */
typedef void (*node_ctrl_cb_t)(const char *cmd, int32_t value);

/* 初始化从机（启动 BLE GATT Server 并广播 XZ-<type>-<id>） */
void ble_node_init(uint8_t node_type, uint32_t node_id, node_ctrl_cb_t ctrl_cb);

/* 上报状态（Notify，data 自定义内容） */
void ble_node_report_status(const uint8_t *data, int len);

/* 上报传感器数据（temp/humi/air/light 各4字节 float） */
void ble_node_report_sensor(float temp, float humi, float air, float light);

#ifdef __cplusplus
}
#endif

#endif /* BLE_NODE_H */
