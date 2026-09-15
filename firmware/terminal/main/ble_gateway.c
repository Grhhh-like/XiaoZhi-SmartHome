/*
 * ble_gateway.c - 蓝牙 BLE 网关实现
 * 小智·智家 XiaoZhi SmartHome
 *
 * 实现 BLE Central 核心逻辑：
 *  - NimBLE 主机栈：扫描 XZ-* 广播 → 连接 → 发现 GATT 服务
 *  - 控制特征值写入（Write Without Response）
 *  - 状态特征值订阅（Notify），缓存传感器最新值
 *  - 掉线自动重连
 *
 * 注意：完整实现需引入 NimBLE 主机 API（esp-nimble 组件）。
 * 本文件给出模块骨架与协议约定，可按官方文档补充回调细节。
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ble_gateway.h"

static const char *TAG = "ble_gateway";

/* 已连接节点表（最多 8 个自制节点） */
#define BLE_GW_MAX_NODES 8

typedef struct {
    bool     in_use;
    char     device[24];
    char     addr_str[18];
    uint16_t conn_handle;
    uint16_t ctrl_handle;  /* 控制特征值句柄 */
    uint16_t state_handle; /* 状态特征值句柄 */
    float    sensor_temp;
    float    sensor_humi;
    float    sensor_air;
    float    sensor_light;
} ble_node_t;

static ble_node_t s_nodes[BLE_GW_MAX_NODES];

/* ---------- 内部工具 ---------- */

static ble_node_t *node_find(const char *device)
{
    for (int i = 0; i < BLE_GW_MAX_NODES; i++) {
        if (s_nodes[i].in_use && strcmp(s_nodes[i].device, device) == 0) {
            return &s_nodes[i];
        }
    }
    return NULL;
}

static ble_node_t *node_alloc(const char *device)
{
    for (int i = 0; i < BLE_GW_MAX_NODES; i++) {
        if (!s_nodes[i].in_use) {
            ble_node_t *n = &s_nodes[i];
            memset(n, 0, sizeof(*n));
            snprintf(n->device, sizeof(n->device), "%s", device);
            n->in_use = true;
            return n;
        }
    }
    return NULL;
}

static int send_ctrl(ble_node_t *node, const char *cmd, int32_t value)
{
    if (!node || node->conn_handle == 0xFFFF) {
        ESP_LOGW(TAG, "节点 %s 未连接", node ? node->device : "?");
        return -1;
    }

    ble_ctrl_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    snprintf(frame.device, sizeof(frame.device), "%s", node->device);
    snprintf(frame.cmd, sizeof(frame.cmd), "%s", cmd);
    frame.value = value;

    /* TODO: nimble_gattc_write_flat(node->conn_handle, node->ctrl_handle,
     *        (uint8_t *)&frame, sizeof(frame), 0, NULL, NULL); */
    ESP_LOGI(TAG, "BLE 控制 → %s: cmd=%s value=%ld", node->device, cmd, (long)value);
    return 0;
}

/* ---------- 对外 API ---------- */

void ble_gateway_init(void)
{
    ESP_LOGI(TAG, "BLE gateway init (Central mode)");

    /* TODO: nimble 主机栈初始化、注册扫描回调、配置广播过滤 "XZ-" 前缀 */

    memset(s_nodes, 0, sizeof(s_nodes));
    ESP_LOGI(TAG, "等待 BLE 节点广播（XZ-* 前缀，最多 %d 个）", BLE_GW_MAX_NODES);
}

int ble_light_ctrl(const char *device, const char *cmd, int32_t value)
{
    ble_node_t *n = node_find(device);
    return send_ctrl(n, cmd, value);
}

int ble_curtain_ctrl(const char *device, const char *cmd, int32_t value)
{
    ble_node_t *n = node_find(device);
    return send_ctrl(n, cmd, value);
}

int ble_plug_ctrl(const char *device, const char *cmd)
{
    ble_node_t *n = node_find(device);
    return send_ctrl(n, cmd, 0);
}

int ble_sensor_read(const char *node_id, const char *type, float *out_value)
{
    ble_node_t *n = node_find(node_id);
    if (!n || !out_value) {
        return -1;
    }
    if (strcmp(type, "temp") == 0) {
        *out_value = n->sensor_temp;
    } else if (strcmp(type, "humi") == 0) {
        *out_value = n->sensor_humi;
    } else if (strcmp(type, "air") == 0) {
        *out_value = n->sensor_air;
    } else if (strcmp(type, "light") == 0) {
        *out_value = n->sensor_light;
    } else {
        return -1;
    }
    return 0;
}

void ble_gateway_tick(void)
{
    /* TODO: 周期扫描未连接节点、处理重连与状态订阅超时 */
}
