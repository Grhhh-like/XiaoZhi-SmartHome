/*
 * proactive.h - 主动智能引擎
 * 小智·智家 XiaoZhi SmartHome
 * 让小智从"被动应答"走向"主动关怀"：
 *  - 规则引擎（when→if→then）
 *  - 习惯学习（作息统计、异常检测）
 *  - 场景联动（原子动作组合）
 *  - 主动播报（优先级队列、打断恢复）
 */
#ifndef PROACTIVE_H
#define PROACTIVE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 主动提醒条目 */
typedef struct {
    char rule_id[16];     /* 规则ID */
    char message[192];    /* 播报文本 */
    uint8_t priority;     /* 优先级 0-10，10最高 */
} proactive_alert_t;

/* 初始化主动引擎 */
void proactive_init(void);

/* 周期性 tick（建议 1s 调用），内部执行规则检查 */
void proactive_tick(void);

/* 主动播报队列：压入一条提醒（TTS 会打断低优先级播放） */
void proactive_push_alert(const proactive_alert_t *alert);

/* 习惯学习：记录一次交互（设备/命令/是否成功） */
void proactive_log_interaction(const char *device, const char *cmd, bool success);

/* 执行场景联动，成功返回 0 */
int proactive_execute_scene(const char *scene);

/* 注册自定义场景（JSON/YAML 由网关同步） */
void proactive_register_scene(const char *name, const char *actions_json);

#ifdef __cplusplus
}
#endif

#endif /* PROACTIVE_H */
