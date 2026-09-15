/*
 * sensor_fusion.c - 多传感器融合模块实现
 * 小智·智家 XiaoZhi SmartHome
 */
#include <string.h>
#include <stdio.h>
#include "esp_log.h"
#include "sensor_fusion.h"

static const char *TAG = "sensor_fusion";

static env_state_t s_state = {
    .temp = 25.0f, .humi = 50.0f, .air = 400.0f,
    .light = 300.0f, .occupied = false, .ts = 0,
};

/* 环境阈值 */
#define TEMP_HIGH       30.0f
#define TEMP_LOW        16.0f
#define HUMI_HIGH       75.0f
#define AIR_GOOD        800.0f   /* CO2 ppm */
#define AIR_BAD         1500.0f
#define AIR_DANGER      2500.0f

void sensor_fusion_update(const env_state_t *state)
{
    if (!state) {
        return;
    }
    s_state = *state;
    s_state.ts = (uint32_t)(esp_timer_get_time() / 1000);
    ESP_LOGD(TAG, "env: %.1fC %.1f%% air=%.0f lux=%.0f occ=%d",
             s_state.temp, s_state.humi, s_state.air, s_state.light, s_state.occupied);
}

void sensor_fusion_get(env_state_t *out)
{
    if (out) {
        *out = s_state;
    }
}

int sensor_fusion_comfort_score(void)
{
    int score = 100;
    /* 温度舒适区间 22-26 */
    if (s_state.temp < 22.0f || s_state.temp > 26.0f) {
        score -= (int)((s_state.temp < 22.0f ? 22.0f - s_state.temp : s_state.temp - 26.0f) * 5);
    }
    /* 湿度舒适区间 40-60 */
    if (s_state.humi < 40.0f || s_state.humi > 60.0f) {
        score -= (int)((s_state.humi < 40.0f ? 40.0f - s_state.humi : s_state.humi - 60.0f) * 1.5f);
    }
    /* 空气质量 */
    if (s_state.air > AIR_GOOD) {
        score -= (int)((s_state.air - AIR_GOOD) / 50.0f);
    }
    if (score < 0) {
        score = 0;
    }
    if (score > 100) {
        score = 100;
    }
    return score;
}

int sensor_fusion_air_grade(void)
{
    if (s_state.air < AIR_GOOD)      return 0;
    if (s_state.air < AIR_BAD)       return 1;
    if (s_state.air < AIR_DANGER)    return 2;
    return 3;
}

bool sensor_fusion_has_alarm(char *reason, int reason_len)
{
    if (s_state.temp > TEMP_HIGH) {
        snprintf(reason, reason_len, "温度偏高(%.1f°C)", s_state.temp);
        return true;
    }
    if (s_state.temp < TEMP_LOW) {
        snprintf(reason, reason_len, "温度偏低(%.1f°C)", s_state.temp);
        return true;
    }
    if (s_state.humi > HUMI_HIGH) {
        snprintf(reason, reason_len, "湿度偏高(%.1f%%)", s_state.humi);
        return true;
    }
    if (s_state.air > AIR_BAD) {
        snprintf(reason, reason_len, "空气质量差(CO2 %.0fppm)", s_state.air);
        return true;
    }
    return false;
}

const char *sensor_fusion_unit(const char *type)
{
    if (strcmp(type, "temp") == 0)  return "度";
    if (strcmp(type, "humi") == 0)  return "%%";
    if (strcmp(type, "air") == 0)   return "ppm";
    if (strcmp(type, "light") == 0) return "勒克斯";
    return "";
}
