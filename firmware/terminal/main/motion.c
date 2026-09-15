/*
 * motion.c - 移动底盘运动控制实现
 * 小智·智家 XiaoZhi SmartHome
 *
 * 双 N20 减速电机（约 30:1，霍尔编码器 11PPR×30≈330 脉冲/圈）
 * TB6612 驱动：AIN1/AIN2 + PWMA（左），BIN1/BIN2 + PWMB（右）
 * 编码器计数（PCNT 外设）→ 速度计算 → PID → PWM 输出
 *
 * 说明：完整 PID/里程计集成需按实际底盘参数标定；
 * 本文件提供模块骨架与协议约定。
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "motion.h"

static const char *TAG = "motion";

/* 底盘参数（按实际标定） */
#define WHEEL_DIAM_CM   6.5f    /* 轮径 cm */
#define PULSES_PER_REV  330     /* 编码器脉冲/圈 */
#define PWM_FREQ_HZ     20000
#define PWM_RES_BITS    10

/* 引脚（按实际底盘修改） */
#define PIN_PWM_A   GPIO_NUM_15
#define PIN_PWM_B   GPIO_NUM_16
#define PIN_AIN1    GPIO_NUM_17
#define PIN_AIN2    GPIO_NUM_18
#define PIN_BIN1    GPIO_NUM_21
#define PIN_BIN2    GPIO_NUM_47
#define PIN_ENC_L_A GPIO_NUM_5
#define PIN_ENC_L_B GPIO_NUM_6
#define PIN_ENC_R_A GPIO_NUM_7
#define PIN_ENC_R_B GPIO_NUM_8

static motion_state_t s_state = {.mode = MOTION_STOP, .speed = 0};

/* PID 参数（占位，需整定） */
#define KP 1.2f
#define KI 0.15f
#define KD 0.05f

static float s_pid_integral = 0;
static float s_last_error = 0;
static int16_t s_target_rpm = 0;

void motion_init(void)
{
    ESP_LOGI(TAG, "motion init (differential drive)");

    /* TODO:
     *  - LEDC 配置 PWM_A/PWM_B（20kHz, 10bit）
     *  - GPIO 配置 AIN/BIN 方向引脚
     *  - PCNT 配置双路编码器计数
     *  - 定时器（10ms）周期回调 → motion_tick()
     */
    s_state.mode = MOTION_STOP;
    s_state.speed = 0;
    ESP_LOGI(TAG, "motion ready: wheel=%.1fcm pulses/rev=%d", WHEEL_DIAM_CM, PULSES_PER_REV);
}

static void set_motor(int16_t pwm_left, int16_t pwm_right)
{
    /* TODO: 设置方向引脚与 PWM 占空比（含死区补偿） */
    ESP_LOGD(TAG, "motor L=%d R=%d", pwm_left, pwm_right);
}

void motion_set(motion_mode_t mode, int16_t speed)
{
    if (speed < 0) speed = 0;
    if (speed > 30) speed = 30;

    s_state.mode = mode;
    s_state.speed = speed;

    /* 目标转速：v(cm/s) → rpm（周长=πD≈20.4cm） */
    float cm_per_rev = 3.14159f * WHEEL_DIAM_CM;
    s_target_rpm = (int16_t)(speed * 60.0f / cm_per_rev);

    switch (mode) {
    case MOTION_FORWARD:
        set_motor(s_target_rpm, s_target_rpm);
        break;
    case MOTION_BACKWARD:
        set_motor(-s_target_rpm, -s_target_rpm);
        break;
    case MOTION_LEFT:
        set_motor(0, s_target_rpm);
        break;
    case MOTION_RIGHT:
        set_motor(s_target_rpm, 0);
        break;
    case MOTION_SPIN_LEFT:
        set_motor(-s_target_rpm, s_target_rpm);
        break;
    case MOTION_SPIN_RIGHT:
        set_motor(s_target_rpm, -s_target_rpm);
        break;
    case MOTION_STOP:
    default:
        set_motor(0, 0);
        break;
    }
    ESP_LOGI(TAG, "motion set: mode=%d speed=%dcm/s target_rpm=%d", mode, speed, s_target_rpm);
}

void motion_stop(void)
{
    motion_set(MOTION_STOP, 0);
}

void motion_move_cm(int16_t distance_cm, int16_t speed)
{
    /* TODO: 根据里程计闭环移动指定距离后停止 */
    ESP_LOGI(TAG, "move %dcm @ %dcm/s", distance_cm, speed);
    motion_set(distance_cm >= 0 ? MOTION_FORWARD : MOTION_BACKWARD, speed);
}

void motion_rotate_deg(int16_t deg, int16_t speed)
{
    /* TODO: IMU/里程计闭环旋转指定角度（正=左转） */
    ESP_LOGI(TAG, "rotate %ddeg @ %dcm/s", deg, speed);
    motion_set(deg >= 0 ? MOTION_SPIN_LEFT : MOTION_SPIN_RIGHT, speed);
}

const motion_state_t *motion_get_state(void)
{
    return &s_state;
}

void motion_tick(void)
{
    /* TODO:
     *  - 读取 PCNT 计数，计算左右轮实时转速
     *  - PID 计算 → 修正 PWM
     *  - 更新 s_state（rpm/里程计）
     */
}
