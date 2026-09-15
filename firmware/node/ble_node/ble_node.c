/*
 * ble_node.c - 智能节点公共模块实现（BLE 从机 / GATT Server）
 * 小智·智家 XiaoZhi SmartHome
 *
 * 基于 NimBLE 主机栈（esp-nimble）实现 GATT Server：
 *  - 广播名 XZ-<type>-<id>，例如 XZ-light-living（type 与 id 由 init 参数决定）
 *  - 服务/特征值 UUID 与终端 ble_gateway 一致
 *  - 控制特征值：Write（接收 ble_node_ctrl_t）
 *  - 状态特征值：Notify（上报状态/传感器）
 *
 * 说明：完整实现需在工程中启用 NimBLE 并注册回调；
 * 本文件给出协议骨架与事件流，可按官方 NimBLE 文档补齐细节。
 */
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "ble_node.h"

static const char *TAG = "ble_node";

static uint8_t s_node_type = NODE_SENSOR;
static uint32_t s_node_id = 0;
static node_ctrl_cb_t s_ctrl_cb = NULL;

/* 节点广播名（XZ-<type>-<id>） */
static char s_adv_name[24];

static void build_adv_name(void)
{
    /* type 与 id 压缩进广播名，例如 XZ-1-00000001 */
    snprintf(s_adv_name, sizeof(s_adv_name), "XZ-%02X-%08X",
             (unsigned)s_node_type, (unsigned)s_node_id);
}

void ble_node_init(uint8_t node_type, uint32_t node_id, node_ctrl_cb_t ctrl_cb)
{
    s_node_type = node_type;
    s_node_id = node_id;
    s_ctrl_cb = ctrl_cb;
    build_adv_name();

    /* TODO: NimBLE 初始化
     *  - nimble_port_init / ble_hs_init
     *  - 注册 GATT 服务（BLE_NODE_SERVICE_UUID，两个特征值）
     *  - 控制特征值回调：收到写请求 → 解析 ble_node_ctrl_t → 调 s_ctrl_cb(cmd, value)
     *  - 状态特征值 CCCD 订阅管理
     *  - 启动广播：ble_gap_adv_start，广播名 s_adv_name
     */

    ESP_LOGI(TAG, "BLE node ready: type=%d id=%08X adv=%s",
             (unsigned)node_type, (unsigned)node_id, s_adv_name);
}

void ble_node_report_status(const uint8_t *data, int len)
{
    /* TODO: 状态特征值 Notify（需先确认客户端已订阅） */
    ESP_LOGI(TAG, "notify status: len=%d", len);
}

void ble_node_report_sensor(float temp, float humi, float air, float light)
{
    uint8_t buf[16];
    memcpy(buf,      &temp,  4);
    memcpy(buf + 4,  &humi,  4);
    memcpy(buf + 8,  &air,   4);
    memcpy(buf + 12, &light, 4);
    ble_node_report_status(buf, sizeof(buf));
}
