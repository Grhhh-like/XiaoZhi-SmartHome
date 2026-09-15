/*
 * ir_remote.h - 红外遥控发射模块
 * 小智·智家 XiaoZhi SmartHome
 * 支持 NEC 协议与 RAW 码，38kHz 载波，用于控制空调/电视/风扇等老家电
 */
#ifndef IR_REMOTE_H
#define IR_REMOTE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IR_CARRIER_HZ     38000  /* 载波频率 */
#define IR_GPIO_DEFAULT   17     /* 默认发射引脚 */

/* NEC 码定义（地址, 命令） */
typedef struct {
    uint8_t addr;
    uint8_t cmd;
} nec_code_t;

/* 红外命令条目 */
typedef struct {
    char    brand[16];   /* 品牌: gree/media/haier/generic */
    char    device[16];  /* ac/tv/fan */
    char    cmd[16];     /* power_on/power_off/temp_26/mode_cool/... */
    nec_code_t nec;      /* NEC 编码 */
    bool    is_raw;      /* 是否为 RAW 码 */
    const uint16_t *raw; /* RAW 码数组 */
    uint16_t raw_len;
} ir_cmd_entry_t;

/* 初始化红外发射 */
void ir_remote_init(void);

/* 发送 NEC 码 */
void ir_send_nec(uint8_t addr, uint8_t cmd);

/* 发送 RAW 码 */
void ir_send_raw(const uint16_t *buf, uint16_t len);

/* 按品牌/设备/命令查询并发送，返回 0 成功 */
int ir_send_command(const char *brand, const char *device, const char *cmd);

/* 注册自定义码库条目（学习录入） */
void ir_register_code(const ir_cmd_entry_t *entry);

/* 内置码库条目数 */
int ir_code_count(void);

#ifdef __cplusplus
}
#endif

#endif /* IR_REMOTE_H */
