/*
 * rf433.c - 433MHz 射频发射模块实现
 * 小智·智家 XiaoZhi SmartHome
 * 无线插座常见编码：同步头 + 地址位 + 按键位 + 重复帧
 */
#include <string.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "rf433.h"

static const char *TAG = "rf433";

static int s_rf_gpio = RF_GPIO_DEFAULT;

#define RF_PULSE_US     400  /* 基本脉冲宽度 */
#define RF_REPEAT       8    /* 帧重复次数 */
#define RF_TX_TIMEOUT   10   /* 发射超时 ms */

void rf433_init(void)
{
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << s_rf_gpio),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io);
    gpio_set_level(s_rf_gpio, 0);
    ESP_LOGI(TAG, "433MHz TX ready on GPIO%d", s_rf_gpio);
}

static inline void pulse_us(uint32_t us)
{
    esp_rom_delay_us(us);
}

static void send_bit(int bit)
{
    if (bit) {
        gpio_set_level(s_rf_gpio, 1);
        pulse_us(RF_PULSE_US);
        gpio_set_level(s_rf_gpio, 0);
        pulse_us(RF_PULSE_US * 3);
    } else {
        gpio_set_level(s_rf_gpio, 1);
        pulse_us(RF_PULSE_US * 3);
        gpio_set_level(s_rf_gpio, 0);
        pulse_us(RF_PULSE_US);
    }
}

static void send_sync(void)
{
    gpio_set_level(s_rf_gpio, 1);
    pulse_us(RF_PULSE_US * 9); /* 同步长脉冲 */
    gpio_set_level(s_rf_gpio, 0);
    pulse_us(RF_PULSE_US);
}

void rf_send_bits(uint32_t code, uint8_t bits)
{
    for (int rep = 0; rep < RF_REPEAT; rep++) {
        send_sync();
        for (int i = bits - 1; i >= 0; i--) {
            send_bit((code >> i) & 1);
        }
        gpio_set_level(s_rf_gpio, 0);
        pulse_us(RF_PULSE_US * 30); /* 帧间隔 */
    }
    ESP_LOGD(TAG, "RF sent: code=%08X bits=%d", code, bits);
}

void rf_send_key(int remote_id, const char *cmd)
{
    if (!cmd || (strcmp(cmd, "on") != 0 && strcmp(cmd, "off") != 0)) {
        ESP_LOGW(TAG, "invalid rf cmd: %s", cmd ? cmd : "null");
        return;
    }

    /* 编码格式：4bit 遥控ID + 1bit 键值 + 1bit 固定 */
    uint32_t code = ((uint32_t)(remote_id & 0x0F)) << 2;
    code |= (strcmp(cmd, "on") == 0) ? 0x2 : 0x0;
    code |= 0x1; /* 固定位 */

    rf_send_bits(code, 6);
    ESP_LOGI(TAG, "RF key: remote=%d cmd=%s", remote_id, cmd);
}
