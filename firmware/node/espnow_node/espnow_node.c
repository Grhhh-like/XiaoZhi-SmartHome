/*
 * espnow_node.c - 智能节点公共模块实现
 * 小智·智家 XiaoZhi SmartHome
 */
#include <string.h>
#include "esp_log.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "espnow_node.h"

static const char *TAG = "espnow_node";

static uint8_t s_node_type = NODE_SENSOR;
static uint32_t s_node_id = 0;
static node_ctrl_cb_t s_ctrl_cb = NULL;
static uint8_t s_master[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static void send_frame(uint8_t type, uint8_t cmd, const uint8_t *data, int len)
{
    espnow_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    frame.frame_type = type;
    frame.node_type = s_node_type;
    frame.node_id = s_node_id;
    frame.cmd = cmd;
    if (data && len > 0) {
        memcpy(frame.data, data, len > (int)sizeof(frame.data) ? (int)sizeof(frame.data) : len);
    }
    esp_now_send(s_master, (const uint8_t *)&frame, sizeof(frame));
}

static void on_recv(const esp_now_recv_info_t *info, const uint8_t *data, int len)
{
    if (len < (int)sizeof(espnow_frame_t)) {
        return;
    }
    const espnow_frame_t *frame = (const espnow_frame_t *)data;

    if (frame->node_type != s_node_type && frame->node_type != 0xFF) {
        return; /* 不是发给本类型节点 */
    }
    if (frame->node_id != s_node_id && frame->node_id != 0xFFFFFFFF) {
        return; /* 不是发给本节点 */
    }

    switch (frame->frame_type) {
    case FRAME_CTRL:
        if (s_ctrl_cb) {
            s_ctrl_cb(frame->cmd, frame->data, sizeof(frame->data));
        }
        /* 应答 */
        send_frame(FRAME_STATUS, CMD_ACK, NULL, 0);
        break;
    case FRAME_DISCOVER:
        send_frame(FRAME_STATUS, CMD_ACK, NULL, 0);
        break;
    default:
        break;
    }
}

void espnow_node_init(uint8_t node_type, uint32_t node_id,
                      const uint8_t master_mac[6],
                      node_ctrl_cb_t ctrl_cb)
{
    s_node_type = node_type;
    s_node_id = node_id;
    s_ctrl_cb = ctrl_cb;
    if (master_mac) {
        memcpy(s_master, master_mac, 6);
    }

    ESP_ERROR_CHECK(esp_netif_init());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_recv));

    esp_now_peer_info_t peer = {.channel = 0, .ifidx = ESP_IF_WIFI_STA, .encrypt = false};
    memcpy(peer.peer_addr, s_master, 6);
    esp_now_add_peer(&peer);

    ESP_LOGI(TAG, "node ready: type=%d id=%08X master=%02X:%02X:%02X:%02X:%02X:%02X",
             node_type, (unsigned)node_id,
             s_master[0], s_master[1], s_master[2], s_master[3], s_master[4], s_master[5]);
}

void espnow_node_report_status(const uint8_t *data, int len)
{
    send_frame(FRAME_STATUS, CMD_ACK, data, len);
}

void espnow_node_report_sensor(float temp, float humi, float air, float light)
{
    uint8_t buf[16];
    memcpy(buf,      &temp,  4);
    memcpy(buf + 4,  &humi,  4);
    memcpy(buf + 8,  &air,   4);
    memcpy(buf + 12, &light, 4);
    send_frame(FRAME_SENSOR, CMD_QUERY, buf, sizeof(buf));
}
