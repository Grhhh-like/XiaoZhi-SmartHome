/*
 * main.c - 智能灯节点
 * 小智·智家 XiaoZhi SmartHome
 * ESP32-C3 + MOSFET 调光 + WS2812 灯带
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "espnow_node.h"

static const char *TAG = "light_node";

#define NODE_ID       0x00000001
#define PIN_DIMMER    GPIO_NUM_3   /* MOSFET 调光 */
#define PIN_WS2812    GPIO_NUM_4   /* RGB 灯带 */
#define LEDC_CH       0
#define LEDC_TIMER    0

static uint8_t s_brightness = 100; /* 0-100 */

static void set_brightness(uint8_t v)
{
    s_brightness = (v > 100) ? 100 : v;
    uint32_t duty = (s_brightness * 8191) / 100; /* 13bit */
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    ESP_LOGI(TAG, "brightness -> %d%%", s_brightness);
}

/* 节点控制回调 */
static void on_ctrl(uint8_t cmd, const uint8_t *data, int len)
{
    switch (cmd) {
    case CMD_ON:
        set_brightness(100);
        break;
    case CMD_OFF:
        set_brightness(0);
        break;
    case CMD_SET_VALUE:
        if (len >= 1) {
            set_brightness(data[0]);
        }
        break;
    default:
        break;
    }
}

void app_main(void)
{
    /* PWM 调光初始化 */
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t ch = {
        .gpio_num = PIN_DIMMER,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&ch);

    /* ESP-NOW 从机 */
    espnow_node_init(NODE_LIGHT, NODE_ID, NULL, on_ctrl);

    ESP_LOGI(TAG, "light node ready, id=%08X", NODE_ID);

    /* 呼吸自检效果 */
    for (int i = 0; i < 5; i++) {
        for (int v = 0; v <= 100; v += 5) {
            set_brightness(v);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        for (int v = 100; v >= 0; v -= 5) {
            set_brightness(v);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    set_brightness(50);
}
