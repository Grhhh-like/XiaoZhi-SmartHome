/*
 * espnow_node.h - 智能节点公共模块（ESP-NOW 从机）
 * 小智·智家 XiaoZhi SmartHome
 * 节点侧统一协议：接收控制帧、上报状态/传感器数据
 */
#ifndef ESPNOW_NODE_H
#define ESPNOW_NODE_H

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

/* 帧类型 */
#define FRAME_CTRL     0x01
#define FRAME_STATUS   0x02
#define FRAME_SENSOR   0x03
#define FRAME_DISCOVER 0x04

/* 命令 */
#define CMD_ON         0x01
#define CMD_OFF        0x02
#define CMD_SET_VALUE  0x03
#define CMD_QUERY      0x10
#define CMD_ACK        0xF0

#pragma pack(push, 1)
typedef struct {
    uint8_t  frame_type;
    uint8_t  node_type;
    uint32_t node_id;
    uint8_t  cmd;
    uint8_t  data[64];
} espnow_frame_t;
#pragma pack(pop)

/* 节点回调：收到控制指令时触发（由各节点实现） */
typedef void (*node_ctrl_cb_t)(uint8_t cmd, const uint8_t *data, int len);

/* 初始化从机（自动注册到主节点 MAC） */
void espnow_node_init(uint8_t node_type, uint32_t node_id,
                      const uint8_t master_mac[6],
                      node_ctrl_cb_t ctrl_cb);

/* 上报状态（data: 自定义, len 字节） */
void espnow_node_report_status(const uint8_t *data, int len);

/* 上报传感器数据（temp/humi/air/light 各4字节 float） */
void espnow_node_report_sensor(float temp, float humi, float air, float light);

#ifdef __cplusplus
}
#endif

#endif /* ESPNOW_NODE_H */
