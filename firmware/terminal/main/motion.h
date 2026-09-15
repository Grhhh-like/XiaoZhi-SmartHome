/*
 * motion.h - 移动底盘运动控制
 * 小智·智家 XiaoZhi SmartHome
 *
 * 中型移动机器人底盘（双 N20 减速电机差速驱动）：
 *  - TB6612 双路电机驱动，PWM 调速 + 方向控制
 *  - 霍尔编码器测速，PID 闭环速度控制
 *  - 运动指令：前进/后退/左转/右转/停止/速度设置/巡航
 */
#ifndef MOTION_H
#define MOTION_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 运动模式 */
typedef enum {
    MOTION_STOP = 0,
    MOTION_FORWARD,
    MOTION_BACKWARD,
    MOTION_LEFT,
    MOTION_RIGHT,
    MOTION_SPIN_LEFT,   /* 原地左转 */
    MOTION_SPIN_RIGHT,  /* 原地右转 */
} motion_mode_t;

/* 运动状态 */
typedef struct {
    motion_mode_t mode;
    int16_t       speed;      /* 目标速度 cm/s（0-30） */
    float         left_rpm;   /* 左轮实时转速 */
    float         right_rpm;  /* 右轮实时转速 */
    uint32_t      odo_left;   /* 左轮编码器累计 */
    uint32_t      odo_right;  /* 右轮编码器累计 */
} motion_state_t;

/* ---------- API ---------- */

/* 初始化底盘（PWM/GPIO/编码器/定时器） */
void motion_init(void);

/* 设置运动模式与速度（speed: 0-30 cm/s） */
void motion_set(motion_mode_t mode, int16_t speed);

/* 停止 */
void motion_stop(void);

/* 移动指定距离后停止（cm，前进为正） */
void motion_move_cm(int16_t distance_cm, int16_t speed);

/* 原地旋转指定角度（度，正=左转） */
void motion_rotate_deg(int16_t deg, int16_t speed);

/* 获取实时状态 */
const motion_state_t *motion_get_state(void);

/* 周期控制任务（PID 更新，10ms 调用） */
void motion_tick(void);

#ifdef __cplusplus
}
#endif

#endif /* MOTION_H */
