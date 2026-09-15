/*
 * mqtt_bridge.c - MQTT 桥接模块实现
 * 小智·智家 XiaoZhi SmartHome
 */
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_random.h"
#include "mqtt_client.h"
#include "mqtt_bridge.h"

static const char *TAG = "mqtt_bridge";

static esp_mqtt_client_handle_t s_client = NULL;
static bool s_connected = false;

/* 配置：见 main.c，此处为默认值 */
#define MQTT_BROKER_URI "mqtt://192.168.1.10:1883"
#define MQTT_USER       "smart_home"
#define MQTT_PASS       "change_me"

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        s_connected = true;
        ESP_LOGI(TAG, "MQTT connected to %s", MQTT_BROKER_URI);
        break;
    case MQTT_EVENT_DISCONNECTED:
        s_connected = false;
        ESP_LOGW(TAG, "MQTT disconnected, retrying...");
        break;
    case MQTT_EVENT_DATA:
        /* 收到设备状态回读，可转发给主动引擎/语音播报 */
        ESP_LOGD(TAG, "MQTT data: topic=%.*s, data=%.*s",
                 event->topic_len, event->topic,
                 event->data_len, event->data);
        break;
    default:
        break;
    }
}

void mqtt_bridge_init(void)
{
    esp_mqtt_client_config_t cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .credentials.username = MQTT_USER,
        .credentials.authentication.password = MQTT_PASS,
        .session.keepalive = 60,
    };
    s_client = esp_mqtt_client_init(&cfg);
    esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(s_client);
    ESP_LOGI(TAG, "MQTT bridge init: %s", MQTT_BROKER_URI);
}

void mqtt_publish_device(const char *device, const char *cmd, int32_t value)
{
    if (!s_connected) {
        ESP_LOGW(TAG, "MQTT not connected, drop publish %s", device);
        return;
    }
    char topic[128];
    snprintf(topic, sizeof(topic), "smart_home/%s/cmd", device);

    char payload[192];
    snprintf(payload, sizeof(payload),
             "{\"cmd\":\"%s\",\"value\":%d,\"request_id\":\"tz%06u\"}",
             cmd, value, (unsigned)(esp_random() % 1000000));

    esp_mqtt_client_publish(s_client, topic, payload, 0, 1, 0);
    ESP_LOGI(TAG, "publish %s -> %s", topic, payload);
}

void mqtt_subscribe_status(const char *device)
{
    if (!s_connected) {
        return;
    }
    char topic[128];
    snprintf(topic, sizeof(topic), "smart_home/%s/status", device);
    esp_mqtt_client_subscribe(s_client, topic, 1);
}

int mqtt_get_device_status(const char *device, char *buf, int buf_len)
{
    /* 简化实现：最近一次 MQTT_EVENT_DATA 缓存 */
    (void)device; (void)buf; (void)buf_len;
    return -1; /* 未实现缓存时返回未就绪 */
}

bool mqtt_is_connected(void)
{
    return s_connected;
}
