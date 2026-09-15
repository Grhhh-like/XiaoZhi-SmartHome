/*
 * rf433.h - 433MHz 射频发射模块
 * 小智·智家 XiaoZhi SmartHome
 * 使用 FS1000A 模块，支持固定码/学习码无线插座控制
 */
#ifndef RF433_H
#define RF433_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RF_GPIO_DEFAULT 18 /* 数据引脚 */

/* 初始化 433 发射 */
void rf433_init(void);

/* 发送按键：remote_id 为遥控编号(1-8), cmd 为 on/off */
void rf_send_key(int remote_id, const char *cmd);

/* 发送自定义编码 */
void rf_send_bits(uint32_t code, uint8_t bits);

#ifdef __cplusplus
}
#endif

#endif /* RF433_H */
