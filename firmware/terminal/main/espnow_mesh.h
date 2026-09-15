/*
 * espnow_mesh.h - ESP-NOW 组网主节点模块
 * 小智·智家 XiaoZhi SmartHome
 * 与自制智能节点(灯/窗帘/插座/传感器)组网，无路由器直连，低功耗
 */
#ifndef ESPNOW_MESH_H
#define ESPNOW_MESH_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 节点类型 */
typedef enum {
    NODE_LIGHT    = 0x01,
    NODE_CURTAIN  = 0x02,
    NODE_PLUG     = 0x03,
    NODE_SENSOR   = 0x04,
} node_type_t;

/* ESP-NOW 帧格式 (<=236B)：
 * [0]    帧类型
 * [1]    节点类型
 * [2-5]  节点ID
 * [6]    命令
 * [7-...]参数(变长)
 * [尾]   CRC8
 */
#pragma pack(push, 1)
typedef struct {
    uint8_t  frame_type;
    uint8_t  node_type;
    uint32_t node_id;
    uint8_t  cmd;
    uint8_t  data[64];
} espnow_frame_t;
#pragma pack(pop)

/* 帧类型 */
#define FRAME_CTRL     0x01  /* 控制指令 */
#define FRAME_STATUS   0x02  /* 状态回读 */
#define FRAME_SENSOR   0x03  /* 传感器数据 */
#define FRAME_DISCOVER 0x04  /* 节点发现 */

/* 命令码 */
#define CMD_ON         0x01
#define CMD_OFF        0x02
#define CMD_SET_VALUE  0x03  /* 调光/百分比 */
#define CMD_QUERY      0x10  /* 查询传感器 */
#define CMD_ACK        0xF0  /* 应答 */

/* 初始化 ESP-NOW 主节点 */
void espnow_mesh_init(void);

/* 控制灯节点 */
void espnow_light_ctrl(const char *device, const char *cmd, int32_t value);

/* 控制窗帘节点 */
void espnow_curtain_ctrl(const char *device, const char *cmd, int32_t value);

/* 控制插座节点 */
void espnow_plug_ctrl(const char *device, const char *cmd);

/* 读取传感器节点，成功返回 0 */
int espnow_sensor_read(const char *node_id, const char *type, float *out);

/* 节点发现（广播，收集在线节点） */
int espnow_discover_nodes(char *buf, int buf_len);

/* 主节点 MAC（从机需要配置） */
void espnow_get_master_mac(uint8_t mac[6]);

#ifdef __cplusplus
}
#endif

#endif /* ESPNOW_MESH_H */
