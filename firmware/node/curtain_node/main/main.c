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
#include "ble_node.h"

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
    bool hit_limit = false;

    for (int i = 0; i < steps && s_running; i++) {
        /* 限位检测（提前结束，不覆盖实测位置） */
        if (dir > 0 && gpio_get_level(PIN_LIMIT_OPEN) == 1) {
            s_position = 100;
            hit_limit = true;
            break;
        }
        if (dir < 0 && gpio_get_level(PIN_LIMIT_CLOSE) == 1) {
            s_position = 0;
            hit_limit = true;
            break;
        }
        static int seq_idx = 0;
        step(seq_idx);
        seq_idx = (seq_idx + dir + 8) % 8;
        vTaskDelay(pdMS_TO_TICKS(3));
    }
    motor_off();
    if (!hit_limit && s_running) {
        s_position = target;
    }
    s_running = false;
    ESP_LOGI(TAG, "curtain position -> %d%%%s", s_position, hit_limit ? " (限位)" : "");
}

static void on_ctrl(const char *cmd, int32_t value)
{
    if (!cmd) {
        return;
    }
    if (strcmp(cmd, "open") == 0) {
        move_to(100);
    } else if (strcmp(cmd, "close") == 0) {
        move_to(0);
    } else if (strcmp(cmd, "stop") == 0) {
        s_running = false;
        motor_off();
    } else if (strcmp(cmd, "set_position") == 0) {
        /* 钳制非法值（含负数转 uint8_t 的场景） */
        if (value < 0) value = 0;
        if (value > 100) value = 100;
        move_to((uint8_t)value);
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

    ble_node_init(NODE_CURTAIN, NODE_ID, on_ctrl);
    ESP_LOGI(TAG, "curtain node ready, id=%08X", NODE_ID);
}
