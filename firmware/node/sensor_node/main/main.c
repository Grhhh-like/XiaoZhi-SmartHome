/*
 * main.c - 环境传感器节点
 * 小智·智家 XiaoZhi SmartHome
 * ESP32-C3 + SHT40(温湿度) + SGP40(空气质量) + BH1750(光照)
 * 周期上报，低功耗
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "ble_node.h"

static const char *TAG = "sensor_node";

#define NODE_ID        0x00000004
#define I2C_PORT       I2C_NUM_0
#define PIN_SDA        GPIO_NUM_3
#define PIN_SCL        GPIO_NUM_4

/* 传感器 I2C 地址 */
#define SHT40_ADDR     0x44
#define SGP40_ADDR     0x59
#define BH1750_ADDR    0x23

#define REPORT_INTERVAL_S 30

static void i2c_init(void)
{
    i2c_config_t cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PIN_SDA,
        .scl_io_num = PIN_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(I2C_PORT, &cfg);
    i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0, NULL);
}

static float read_temperature_humidity(float *humi)
{
    /* SHT40: 发送测量命令 0xFD, 读 6 字节 */
    uint8_t cmd = 0xFD;
    i2c_master_write_to_device(I2C_PORT, SHT40_ADDR, &cmd, 1, 100 / portTICK_PERIOD_MS);
    vTaskDelay(pdMS_TO_TICKS(20));

    uint8_t buf[6];
    i2c_master_read_from_device(I2C_PORT, SHT40_ADDR, buf, 6, 100 / portTICK_PERIOD_MS);

    /* 温湿度计算公式: -45 + 175 * (raw / 65535) */
    uint16_t raw_t = (buf[0] << 8) | buf[1];
    uint16_t raw_h = (buf[3] << 8) | buf[4];
    *humi = -6.0f + 125.0f * (raw_h / 65535.0f);
    if (*humi < 0) *humi = 0;
    if (*humi > 100) *humi = 100;
    return -45.0f + 175.0f * (raw_t / 65535.0f);
}

static float read_light(void)
{
    /* BH1750: 连续高分辨率模式 0x10 */
    uint8_t cmd = 0x10;
    i2c_master_write_to_device(I2C_PORT, BH1750_ADDR, &cmd, 1, 100 / portTICK_PERIOD_MS);
    vTaskDelay(pdMS_TO_TICKS(180));

    uint8_t buf[2];
    i2c_master_read_from_device(I2C_PORT, BH1750_ADDR, buf, 2, 100 / portTICK_PERIOD_MS);
    uint16_t raw = (buf[0] << 8) | buf[1];
    return raw / 1.2f; /* 分辨率 1.2 lux/bit */
}

static float read_air(void)
{
    /* SGP40: VOC 指数（演示简化，实际需校准算法） */
    uint8_t cmd[2] = {0x26, 0x0F};
    i2c_master_write_to_device(I2C_PORT, SGP40_ADDR, cmd, 2, 100 / portTICK_PERIOD_MS);
    vTaskDelay(pdMS_TO_TICKS(10));
    uint8_t buf[2];
    i2c_master_read_from_device(I2C_PORT, SGP40_ADDR, buf, 2, 100 / portTICK_PERIOD_MS);
    uint16_t raw = (buf[0] << 8) | buf[1];
    /* 原始值映射到 CO2 当量(ppm) 简化 */
    return 400.0f + (raw / 65535.0f) * 2000.0f;
}

static void sample_and_report(void)
{
    float humi = 50.0f;
    float temp = read_temperature_humidity(&humi);
    float air = read_air();
    float light = read_light();

    ble_node_report_sensor(temp, humi, air, light);
    ESP_LOGI(TAG, "report: %.1fC %.1f%% air=%.0fppm lux=%.0f", temp, humi, air, light);
}

static void on_ctrl(const char *cmd, int32_t value)
{
    (void)value;
    if (!cmd) {
        return;
    }
    if (strcmp(cmd, "query") == 0) {
        sample_and_report();
    }
}

void app_main(void)
{
    i2c_init();
    ble_node_init(NODE_SENSOR, NODE_ID, on_ctrl);
    ESP_LOGI(TAG, "sensor node ready, id=%08X, interval=%ds", NODE_ID, REPORT_INTERVAL_S);

    while (1) {
        sample_and_report();
        vTaskDelay(pdMS_TO_TICKS(REPORT_INTERVAL_S * 1000));
    }
}
