/*
 * offline_cmd.c - 离线命令词引擎实现
 * 小智·智家 XiaoZhi SmartHome
 */
#include <string.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "offline_cmd.h"
#include "ble_gateway.h"

static const char *TAG = "offline_cmd";

/* 默认离线命令词表 */
static const offline_cmd_entry_t s_default_table[] = {
    {"打开客厅灯", "living_room_light", "on",  0},
    {"关闭客厅灯", "living_room_light", "off", 0},
    {"打开卧室灯", "bedroom_light",     "on",  0},
    {"关闭卧室灯", "bedroom_light",     "off", 0},
    {"全屋开灯",   "all",               "on",  0},
    {"全屋关灯",   "all",               "off", 0},
    {"打开插座",   "living_room_plug",  "on",  0},
    {"关闭插座",   "living_room_plug",  "off", 0},
};

#define TABLE_COUNT (sizeof(s_default_table) / sizeof(s_default_table[0]))

void offline_cmd_init(void)
{
    ESP_LOGI(TAG, "offline cmd engine init, %d entries", TABLE_COUNT);
}

bool offline_is_active(void)
{
    wifi_ap_record_t ap;
    /* 检查 WiFi 是否已连接（简化：断连视为离线） */
    return esp_wifi_sta_get_ap_info(&ap) != ESP_OK;
}

static void exec_entry(const offline_cmd_entry_t *e)
{
    if (strcmp(e->device, "all") == 0) {
        /* 全屋广播（BLE 逐个控制） */
        ble_light_ctrl("living_room_light", e->cmd, e->value);
        ble_light_ctrl("bedroom_light", e->cmd, e->value);
        ble_light_ctrl("kitchen_light", e->cmd, e->value);
    } else {
        ble_light_ctrl(e->device, e->cmd, e->value);
    }
    ESP_LOGI(TAG, "offline exec: %s %s %s", e->device, e->cmd, e->cmd);
}

bool offline_cmd_process(const char *text)
{
    if (!text || !text[0]) {
        return false;
    }

    /* 精确/包含匹配 */
    for (int i = 0; i < TABLE_COUNT; i++) {
        if (strstr(text, s_default_table[i].keyword)) {
            exec_entry(&s_default_table[i]);
            return true;
        }
    }
    return false;
}
