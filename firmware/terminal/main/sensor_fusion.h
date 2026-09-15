/*
 * sensor_fusion.h - 多传感器融合模块
 * 小智·智家 XiaoZhi SmartHome
 * 汇聚各节点环境数据，输出融合决策（舒适度/空气质量等级/异常标记）
 */
#ifndef SENSOR_FUSION_H
#define SENSOR_FUSION_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float temp;   /* 温度 ℃ */
    float humi;   /* 湿度 % */
    float air;    /* 空气质量指数(CO2 ppm 或 TVOC) */
    float light;  /* 光照 lux */
    bool  occupied; /* 人体存在 */
    uint32_t ts;  /* 时间戳 ms */
} env_state_t;

/* 更新环境状态（由 BLE/MQTT 数据回调调用） */
void sensor_fusion_update(const env_state_t *state);

/* 读取当前融合环境状态 */
void sensor_fusion_get(env_state_t *out);

/* 舒适度评分 0-100 */
int sensor_fusion_comfort_score(void);

/* 空气质量等级: 0优 1良 2差 3危险 */
int sensor_fusion_air_grade(void);

/* 是否有环境异常（温度过高/湿度过高/空气质量差） */
bool sensor_fusion_has_alarm(char *reason, int reason_len);

/* 传感器类型对应的单位 */
const char *sensor_fusion_unit(const char *type);

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_FUSION_H */
