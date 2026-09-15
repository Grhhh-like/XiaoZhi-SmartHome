/*
 * espnow_mesh.c - ESP-NOW 组网主节点实现
 * 小智·智家 XiaoZhi SmartHome
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_random.h"
#include "espnow_mesh.h"

static const char *TAG = "espnow_mesh";

/* 配置：主节点 MAC，从机用此地址发送 */
static uint8_t s_master_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/* 最近一次传感器数据缓存 */
typedef struct {
    uint32_t node_id;
    float    temp;
    float    humi;
    float    air;
    float    light;
    uint32_t ts;
} sensor_cache_t;

#define MAX_SENSOR_CACHE 8
static sensor_cache_t s_sensor_cache[MAX_SENSOR_CACHE];
static int s_sensor_count = 0;

static void send_frame(const espnow_frame_t *frame, const uint8_t *dst)
{
    esp_now_send(dst, (const uint8_t *)frame, sizeof(*frame));
}

static uint8_t crc8(const uint8_t *data, int len)
{
    uint8_t crc = 0;
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc & 0x80) ? (crc << 1) ^ 0x07 : (crc << 1);
        }
    }
    return crc;
}

static void on_recv(const esp_now_recv_info_t *info, const uint8_t *data, int len)
{
    if (len < (int)sizeof(espnow_frame_t)) {
        return;
    }
    const espnow_frame_t *frame = (const espnow_frame_t *)data;

    switch (frame->frame_type) {
    case FRAME_STATUS:
    case FRAME_SENSOR: {
        /* 缓存传感器数据 */
        if (s_sensor_count < MAX_SENSOR_CACHE) {
            sensor_cache_t *c = &s_sensor_cache[s_sensor_count++];
            c->node_id = frame->node_id;
            c->ts = (uint32_t)(esp_timer_get_time() / 1000);
            /* data: temp(4B) humi(4B) air(4B) light(4B) */
            memcpy(&c->temp,  frame->data, 4);
            memcpy(&c->humi,  frame->data + 4, 4);
            memcpy(&c->air,   frame->data + 8, 4);
            memcpy(&c->light, frame->data + 12, 4);
            ESP_LOGI(TAG, "sensor %08X: %.1fC %.1f%% air=%.0f lux=%.0f",
                     (unsigned)frame->node_id, c->temp, c->humi, c->air, c->light);
        }
        break;
    }
    case FRAME_DISCOVER:
        ESP_LOGI(TAG, "node discovered: type=%d id=%08X", frame->node_type, (unsigned)frame->node_id);
        break;
    default:
        break;
    }
}

static void on_send(const uint8_t *mac, esp_now_send_status_t status)
{
    ESP_LOGD(TAG, "send status: %s", status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

void espnow_mesh_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_recv));
    ESP_ERROR_CHECK(esp_now_register_send_cb(on_send));

    /* 注册对端（广播地址） */
    esp_now_peer_info_t peer = {
        .channel = 0,
        .ifidx = ESP_IF_WIFI_STA,
        .encrypt = false,
    };
    memcpy(peer.peer_addr, s_master_mac, 6);
    esp_now_add_peer(&peer);

    ESP_LOGI(TAG, "ESP-NOW master ready");
}

void espnow_get_master_mac(uint8_t mac[6])
{
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
}

static void build_and_send(uint8_t node_type, uint32_t node_id, uint8_t cmd,
                           const uint8_t *data, int data_len)
{
    espnow_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    frame.frame_type = FRAME_CTRL;
    frame.node_type = node_type;
    frame.node_id = node_id;
    frame.cmd = cmd;
    if (data && data_len > 0) {
        memcpy(frame.data, data, data_len > (int)sizeof(frame.data) ? (int)sizeof(frame.data) : data_len);
    }
    /* CRC 追加在 data 末尾（简化） */
    frame.data[data_len] = crc8((const uint8_t *)&frame, sizeof(frame) - 1);
    send_frame(&frame, s_master_mac);
}

static uint32_t id_from_name(const char *device)
{
    /* 设备名哈希为节点ID */
    uint32_t h = 0x811c9dc5;
    const unsigned char *p = (const unsigned char *)device;
    while (*p) {
        h ^= *p++;
        h *= 0x01000193;
    }
    return h;
}

void espnow_light_ctrl(const char *device, const char *cmd, int32_t value)
{
    uint32_t id = id_from_name(device);
    if (strcmp(cmd, "on") == 0) {
        build_and_send(NODE_LIGHT, id, CMD_ON, NULL, 0);
    } else if (strcmp(cmd, "off") == 0) {
        build_and_send(NODE_LIGHT, id, CMD_OFF, NULL, 0);
    } else {
        uint8_t v = (uint8_t)(value < 0 ? 0 : (value > 100 ? 100 : value));
        build_and_send(NODE_LIGHT, id, CMD_SET_VALUE, &v, 1);
    }
}

void espnow_curtain_ctrl(const char *device, const char *cmd, int32_t value)
{
    uint32_t id = id_from_name(device);
    uint8_t data[4];
    if (strcmp(cmd, "open") == 0 || strcmp(cmd, "on") == 0) {
        data[0] = 100;
        build_and_send(NODE_CURTAIN, id, CMD_SET_VALUE, data, 1);
    } else if (strcmp(cmd, "close") == 0 || strcmp(cmd, "off") == 0) {
        data[0] = 0;
        build_and_send(NODE_CURTAIN, id, CMD_SET_VALUE, data, 1);
    } else {
        data[0] = (uint8_t)(value < 0 ? 0 : (value > 100 ? 100 : value));
        build_and_send(NODE_CURTAIN, id, CMD_SET_VALUE, data, 1);
    }
}

void espnow_plug_ctrl(const char *device, const char *cmd)
{
    uint32_t id = id_from_name(device);
    if (strcmp(cmd, "on") == 0) {
        build_and_send(NODE_PLUG, id, CMD_ON, NULL, 0);
    } else {
        build_and_send(NODE_PLUG, id, CMD_OFF, NULL, 0);
    }
}

int espnow_sensor_read(const char *node_id, const char *type, float *out)
{
    uint32_t id = id_from_name(node_id);
    for (int i = 0; i < s_sensor_count; i++) {
        if (s_sensor_cache[i].node_id == id) {
            if (strcmp(type, "temp") == 0)  *out = s_sensor_cache[i].temp;
            else if (strcmp(type, "humi") == 0) *out = s_sensor_cache[i].humi;
            else if (strcmp(type, "air") == 0)  *out = s_sensor_cache[i].air;
            else if (strcmp(type, "light") == 0) *out = s_sensor_cache[i].light;
            else return -1;
            return 0;
        }
    }
    return -1;
}

int espnow_discover_nodes(char *buf, int buf_len)
{
    (void)buf; (void)buf_len;
    return 0;
}
