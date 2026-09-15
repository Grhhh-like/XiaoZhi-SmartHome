/*
 * ir_remote.c - 红外遥控发射模块实现
 * 小智·智家 XiaoZhi SmartHome
 * 使用 RMT 外设生成 38kHz 载波，支持 NEC 与 RAW 码
 */
#include <string.h>
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "ir_remote.h"

static const char *TAG = "ir_remote";

static rmt_channel_handle_t s_tx_channel = NULL;
static int s_ir_gpio = IR_GPIO_DEFAULT;

/* ============ 内置码库（示例：格力空调 NEC 码） ============ */
/* 注意：实际空调码需通过学习录入获得，以下为格式示例 */
static const ir_cmd_entry_t s_builtin_codes[] = {
    /* 格力空调 - 示例码（需实际学习） */
    {.brand = "gree", .device = "ac", .cmd = "power_on",  .nec = {0x00, 0x08}, .is_raw = false, .raw = NULL, .raw_len = 0},
    {.brand = "gree", .device = "ac", .cmd = "power_off", .nec = {0x00, 0x08}, .is_raw = false, .raw = NULL, .raw_len = 0},
    {.brand = "gree", .device = "ac", .cmd = "mode_cool", .nec = {0x00, 0x09}, .is_raw = false, .raw = NULL, .raw_len = 0},
    {.brand = "gree", .device = "ac", .cmd = "temp_16",   .nec = {0x00, 0x10}, .is_raw = false, .raw = NULL, .raw_len = 0},
    {.brand = "gree", .device = "ac", .cmd = "temp_26",   .nec = {0x00, 0x1A}, .is_raw = false, .raw = NULL, .raw_len = 0},
    /* 通用电视 - 示例 */
    {.brand = "generic", .device = "tv", .cmd = "power_on", .nec = {0x00, 0x45}, .is_raw = false, .raw = NULL, .raw_len = 0},
    {.brand = "generic", .device = "tv", .cmd = "power_off", .nec = {0x00, 0x45}, .is_raw = false, .raw = NULL, .raw_len = 0},
    {.brand = "generic", .device = "tv", .cmd = "vol_up", .nec = {0x00, 0x40}, .is_raw = false, .raw = NULL, .raw_len = 0},
    {.brand = "generic", .device = "tv", .cmd = "vol_down", .nec = {0x00, 0x41}, .is_raw = false, .raw = NULL, .raw_len = 0},
};

#define BUILTIN_COUNT (sizeof(s_builtin_codes) / sizeof(s_builtin_codes[0]))

/* 动态注册表（学习录入） */
#define MAX_USER_CODES 32
static ir_cmd_entry_t s_user_codes[MAX_USER_CODES];
static int s_user_count = 0;

void ir_remote_init(void)
{
    rmt_tx_channel_config_t tx_cfg = {
        .gpio_num = s_ir_gpio,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000, /* 1MHz 精度 */
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_cfg, &s_tx_channel));

    rmt_carrier_config_t carrier = {
        .frequency_hz = IR_CARRIER_HZ,
        .duty_cycle = 0.33,
    };
    ESP_ERROR_CHECK(rmt_apply_carrier(s_tx_channel, &carrier));
    ESP_ERROR_CHECK(rmt_enable(s_tx_channel));
    ESP_LOGI(TAG, "IR TX ready on GPIO%d @38kHz", s_ir_gpio);
}

static void send_pulse_us(uint32_t high_us, uint32_t low_us)
{
    /* 单符号发射：高电平载波 + 低电平 */
    rmt_symbol_word_t sym = {
        .duration0 = high_us, .level0 = 1,
        .duration1 = low_us,  .level1 = 0,
    };
    rmt_transmit_config_t cfg = {.loop_count = 0};
    rmt_transmit(s_tx_channel, &sym, sizeof(sym), &cfg);
    rmt_tx_wait_all_done(s_tx_channel, 100);
}

void ir_send_nec(uint8_t addr, uint8_t cmd)
{
    /* NEC 协议：9ms 引导 + 4.5ms 间隔 + 32bit 数据(地址+反码+命令+反码) + 560us 结束 */
    send_pulse_us(9000, 4500);

    uint32_t data = ((uint32_t)addr) | ((uint32_t)(~addr) << 8) |
                    ((uint32_t)cmd << 16) | ((uint32_t)(~cmd) << 24);

    for (int i = 0; i < 32; i++) {
        if (data & (1u << i)) {
            send_pulse_us(560, 1690); /* bit 1 */
        } else {
            send_pulse_us(560, 560);  /* bit 0 */
        }
    }
    send_pulse_us(560, 0); /* 结束位 */
    ESP_LOGD(TAG, "NEC sent: addr=%02X cmd=%02X", addr, cmd);
}

void ir_send_raw(const uint16_t *buf, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        if (i % 2 == 0) {
            send_pulse_us(buf[i], 0); /* 高电平段 */
        } else {
            /* 前一符号的 low 已在符号内；RAW 格式按帧发送 */
        }
    }
    ESP_LOGD(TAG, "RAW sent: %d symbols", len);
}

static const ir_cmd_entry_t *find_entry(const char *brand, const char *device, const char *cmd)
{
    for (int i = 0; i < BUILTIN_COUNT; i++) {
        const ir_cmd_entry_t *e = &s_builtin_codes[i];
        if (strcmp(e->brand, brand) == 0 && strcmp(e->device, device) == 0 &&
            strcmp(e->cmd, cmd) == 0) {
            return e;
        }
    }
    for (int i = 0; i < s_user_count; i++) {
        const ir_cmd_entry_t *e = &s_user_codes[i];
        if (strcmp(e->brand, brand) == 0 && strcmp(e->device, device) == 0 &&
            strcmp(e->cmd, cmd) == 0) {
            return e;
        }
    }
    return NULL;
}

int ir_send_command(const char *brand, const char *device, const char *cmd)
{
    const ir_cmd_entry_t *e = find_entry(brand, device, cmd);
    if (!e) {
        ESP_LOGW(TAG, "code not found: %s/%s/%s", brand, device, cmd);
        return -1;
    }
    if (e->is_raw && e->raw) {
        ir_send_raw(e->raw, e->raw_len);
    } else {
        ir_send_nec(e->nec.addr, e->nec.cmd);
    }
    return 0;
}

void ir_register_code(const ir_cmd_entry_t *entry)
{
    if (!entry || s_user_count >= MAX_USER_CODES) {
        return;
    }
    s_user_codes[s_user_count++] = *entry;
    ESP_LOGI(TAG, "user code registered: %s/%s/%s", entry->brand, entry->device, entry->cmd);
}

int ir_code_count(void)
{
    return BUILTIN_COUNT + s_user_count;
}
