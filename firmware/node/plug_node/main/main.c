/*
 * main.c - 智能插座节点
 * 小智·智家 XiaoZhi SmartHome
 * ESP32-C3 + 继电器 + HLW8032 电能计量
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "ble_node.h"

static const char *TAG = "plug_node";

#define NODE_ID      0x00000003
#define PIN_RELAY    GPIO_NUM_3
#define PIN_RELAY_CT GPIO_NUM_4   /* 继电器控制(高有效) */
#define UART_PORT    UART_NUM_1
#define PIN_HLW_RX   GPIO_NUM_5

static bool s_relay_on = false;

static void relay_set(bool on)
{
    s_relay_on = on;
    gpio_set_level(PIN_RELAY_CT, on ? 1 : 0);
    ESP_LOGI(TAG, "relay -> %s", on ? "ON" : "OFF");
    /* 状态回传（BLE Notify） */
    uint8_t st = on ? 1 : 0;
    ble_node_report_status(&st, 1);
}

static void hlw8032_init(void)
{
    /* HLW8032 计量芯片：UART 9600 8N1 */
    uart_config_t cfg = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(UART_PORT, &cfg);
    uart_set_pin(UART_PORT, UART_PIN_NO_CHANGE, PIN_HLW_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_PORT, 256, 0, 0, NULL, 0);
    ESP_LOGI(TAG, "HLW8032 meter init");
}

static void on_ctrl(const char *cmd, int32_t value)
{
    (void)value;
    if (!cmd) {
        return;
    }
    if (strcmp(cmd, "on") == 0) {
        relay_set(true);
    } else if (strcmp(cmd, "off") == 0) {
        relay_set(false);
    }
}

void app_main(void)
{
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << PIN_RELAY_CT),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io);

    hlw8032_init();
    ble_node_init(NODE_PLUG, NODE_ID, on_ctrl);
    relay_set(false);
    ESP_LOGI(TAG, "plug node ready, id=%08X", NODE_ID);
}
