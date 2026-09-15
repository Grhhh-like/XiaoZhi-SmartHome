/*
 * offline_cmd.h - 离线命令词引擎
 * 小智·智家 XiaoZhi SmartHome
 * 断网时通过本地关键词匹配直接控制核心设备（灯/插座/报警），保证系统可靠性
 */
#ifndef OFFLINE_CMD_H
#define OFFLINE_CMD_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 初始化离线引擎 */
void offline_cmd_init(void);

/* 处理一句本地语音识别结果（ASR本地结果或关键词），
 * 命中则执行控制并返回 true */
bool offline_cmd_process(const char *text);

/* 当前是否离线模式 */
bool offline_is_active(void);

/* 离线命令词表（可配置） */
typedef struct {
    const char *keyword;   /* 关键词 */
    const char *device;    /* 设备名 */
    const char *cmd;       /* 命令 */
    int         value;     /* 参数 */
} offline_cmd_entry_t;

#ifdef __cplusplus
}
#endif

#endif /* OFFLINE_CMD_H */
