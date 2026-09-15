/*
 * main.c - 窗帘电机节点
 * 小智·智家 XiaoZhi SmartHome
 * ESP32-C3 + 28BYJ-48 步进电机 + ULN2003 + 限位开关
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "espnow_node.h"

static const char *TAG = "curtain_node";

#define NODE_ID       0x00000002
#define PIN_IN1       GPIO_NUM_3
#define PIN_IN2       GPIO_NUM_4
#define PIN_IN3       GPIO_NUM_5
#define PIN_IN4       GPIO_NUM_6
#define PIN_LIMIT_OPEN  GPIO_NUM_7   /* 开限位(常开) */
#define PIN_LIMIT_CLOSE GPIO_NUM_8   /* 关限位(常开) */

static const uint8_t STEP_SEQ[8][4] = {
    {1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,1,1,0},
    {0,0,1,0}, {0,0,1,1}, {0,0,0,1}, {1,0,0,1},
};

static uint8_t s_position = 0; /* 0关闭 - 100全开 */
static volatile bool s_running = false;

static void step(uint8_t s)
{
    gpio_set_level(PIN_IN1, STEP_SEQ[s][0]);
    gpio_set_level(PIN_IN2, STEP_SEQ[s][1]);
    gpio_set_level(PIN_IN3, STEP_SEQ[s][2]);
    gpio_set_level(PIN_IN4, STEP_SEQ[s][3]);
}

static void motor_off(void)
{
    gpio_set_level(PIN_IN1, 0);
    gpio_set_level(PIN_IN2, 0);
    gpio_set_level(PIN_IN3, 0);
    gpio_set_level(PIN_IN4, 0);
}

static void move_to(uint8_t target)
{
    if (s_running) {
        return;
    }
    s_running = true;

    /* 简化：以步数近似行程（完整实现需限位校准） */
    int delta = (int)target - (int)s_position;
    int dir = delta > 0 ? 1 : -1;
    int steps = (delta < 0 ? -delta : delta) * 2;

    for (int i = 0; i < steps && s_running; i++) {
        /* 限位检测 */
        if (dir > 0 && gpio_get_level(PIN_LIMIT_OPEN) == 1) {
            s_position = 100;
            break;
        }
        if (dir < 0 && gpio_get_level(PIN_LIMIT_CLOSE) == 1) {
            s_position = 0;
            break;
        }
        static int seq_idx = 0;
        step(seq_idx);
        seq_idx = (seq_idx + dir + 8) % 8;
        vTaskDelay(pdMS_TO_TICKS(3));
    }
    motor_off();
    s_position = target;
    s_running = false;
    ESP_LOGI(TAG, "curtain position -> %d%%", s_position);
}

static void on_ctrl(uint8_t cmd, const uint8_t *data, int len)
{
    switch (cmd) {
    case CMD_ON:       /* 打开 */
        move_to(100);
        break;
    case CMD_OFF:      /* 关闭 */
        move_to(0);
        break;
    case CMD_SET_VALUE:
        if (len >= 1) {
            move_to(data[0] > 100 ? 100 : data[0]);
        }
        break;
    default:
        break;
    }
}

void app_main(void)
{
    gpio_config_t out = {
        .pin_bit_mask = (1ULL << PIN_IN1) | (1ULL << PIN_IN2) |
                        (1ULL << PIN_IN3) | (1ULL << PIN_IN4),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&out);

    gpio_config_t in = {
        .pin_bit_mask = (1ULL << PIN_LIMIT_OPEN) | (1ULL << PIN_LIMIT_CLOSE),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&in);

    espnow_node_init(NODE_CURTAIN, NODE_ID, NULL, on_ctrl);
    ESP_LOGI(TAG, "curtain node ready, id=%08X", NODE_ID);
}
