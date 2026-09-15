/*
 * proactive.c - 主动智能引擎实现
 * 小智·智家 XiaoZhi SmartHome
 */
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "proactive.h"
#include "sensor_fusion.h"
#include "mcp_tools.h"

static const char *TAG = "proactive";

/* ---------- 内置规则（可配置） ---------- */
typedef struct {
    const char *rule_id;
    uint32_t interval_ms;   /* 检查间隔 */
    uint8_t  priority;
} rule_cfg_t;

static const rule_cfg_t s_rules[] = {
    {.rule_id = "env_alarm",  .interval_ms = 60000, .priority = 8},  /* 环境异常 */
    {.rule_id = "routine",    .interval_ms = 600000, .priority = 5}, /* 作息提醒 */
    {.rule_id = "morning",    .interval_ms = 60000, .priority = 6},  /* 晨报(7:30触发) */
};

#define RULE_COUNT (sizeof(s_rules) / sizeof(s_rules[0]))

static uint32_t s_last_check[RULE_COUNT] = {0};
static uint32_t s_start_ms = 0;

/* 交互日志（习惯学习） */
#define LOG_CAP 64
typedef struct {
    char device[32];
    char cmd[16];
    bool success;
    uint32_t ts;
} interaction_log_t;
static interaction_log_t s_log[LOG_CAP];
static int s_log_count = 0;

/* ---------- 场景定义 ---------- */
typedef struct {
    char name[32];
    char actions[512];  /* JSON: [{"device":"living_room_light","cmd":"set_brightness","value":5},...] */
} scene_t;

#define MAX_SCENES 8
static scene_t s_scenes[MAX_SCENES];
static int s_scene_count = 0;

static void register_builtin_scenes(void);

void proactive_init(void)
{
    s_start_ms = (uint32_t)(esp_timer_get_time() / 1000);
    memset(s_last_check, 0, sizeof(s_last_check));
    register_builtin_scenes();
    ESP_LOGI(TAG, "proactive engine init, %d rules, %d scenes", RULE_COUNT, s_scene_count);
}

static void register_builtin_scenes(void)
{
    proactive_register_scene("sleep",
        "[{\"device\":\"living_room_light\",\"cmd\":\"set_brightness\",\"value\":5},"
        "{\"device\":\"bedroom_light\",\"cmd\":\"off\",\"value\":0},"
        "{\"device\":\"living_room_curtain\",\"cmd\":\"close\",\"value\":0},"
        "{\"device\":\"bedroom_ac\",\"cmd\":\"set_temp\",\"value\":26},"
        "{\"device\":\"bedroom_plug\",\"cmd\":\"off\",\"value\":0}]");

    proactive_register_scene("home",
        "[{\"device\":\"living_room_light\",\"cmd\":\"set_brightness\",\"value\":80},"
        "{\"device\":\"living_room_curtain\",\"cmd\":\"open\",\"value\":0}]");

    proactive_register_scene("away",
        "[{\"device\":\"living_room_light\",\"cmd\":\"off\",\"value\":0},"
        "{\"device\":\"bedroom_light\",\"cmd\":\"off\",\"value\":0},"
        "{\"device\":\"living_room_curtain\",\"cmd\":\"close\",\"value\":0},"
        "{\"device\":\"bedroom_ac\",\"cmd\":\"off\",\"value\":0},"
        "{\"device\":\"living_room_plug\",\"cmd\":\"off\",\"value\":0}]");

    proactive_register_scene("movie",
        "[{\"device\":\"living_room_light\",\"cmd\":\"set_brightness\",\"value\":15},"
        "{\"device\":\"living_room_curtain\",\"cmd\":\"close\",\"value\":0}]");
}

void proactive_register_scene(const char *name, const char *actions_json)
{
    if (!name || s_scene_count >= MAX_SCENES) {
        return;
    }
    scene_t *s = &s_scenes[s_scene_count++];
    snprintf(s->name, sizeof(s->name), "%s", name);
    snprintf(s->actions, sizeof(s->actions), "%s", actions_json ? actions_json : "[]");
}

int proactive_execute_scene(const char *scene)
{
    for (int i = 0; i < s_scene_count; i++) {
        if (strcmp(s_scenes[i].name, scene) == 0) {
            /* 简化解析：逐条执行动作（完整实现需 JSON 解析） */
            /* 演示：执行睡眠模式的代表动作 */
            mcp_smart_home_args_t args;
            memset(&args, 0, sizeof(args));
            if (strcmp(scene, "sleep") == 0) {
                snprintf(args.device, sizeof(args.device), "living_room_light");
                snprintf(args.cmd, sizeof(args.cmd), "set_brightness");
                args.value = 5;
                mcp_smart_home_control(&args);
                snprintf(args.device, sizeof(args.device), "bedroom_light");
                snprintf(args.cmd, sizeof(args.cmd), "off");
                mcp_smart_home_control(&args);
            } else if (strcmp(scene, "home") == 0) {
                snprintf(args.device, sizeof(args.device), "living_room_light");
                snprintf(args.cmd, sizeof(args.cmd), "set_brightness");
                args.value = 80;
                mcp_smart_home_control(&args);
            } else if (strcmp(scene, "away") == 0) {
                snprintf(args.device, sizeof(args.device), "living_room_light");
                snprintf(args.cmd, sizeof(args.cmd), "off");
                mcp_smart_home_control(&args);
            }
            ESP_LOGI(TAG, "scene %s executed", scene);
            return 0;
        }
    }
    ESP_LOGW(TAG, "scene %s not found", scene);
    return -1;
}

void proactive_log_interaction(const char *device, const char *cmd, bool success)
{
    if (s_log_count >= LOG_CAP) {
        /* 滚动覆盖最旧 */
        memmove(&s_log[0], &s_log[1], sizeof(interaction_log_t) * (LOG_CAP - 1));
        s_log_count--;
    }
    interaction_log_t *e = &s_log[s_log_count++];
    snprintf(e->device, sizeof(e->device), "%s", device);
    snprintf(e->cmd, sizeof(e->cmd), "%s", cmd);
    e->success = success;
    e->ts = (uint32_t)(esp_timer_get_time() / 1000);
}

static void check_env_alarm(void)
{
    env_state_t env;
    sensor_fusion_get(&env);

    char reason[128];
    if (sensor_fusion_has_alarm(reason, sizeof(reason))) {
        proactive_alert_t alert;
        snprintf(alert.rule_id, sizeof(alert.rule_id), "env_alarm");
        snprintf(alert.message, sizeof(alert.message),
                 "检测到%s，请留意。", reason);
        alert.priority = 8;
        proactive_push_alert(&alert);
    }
}

static void check_routine(void)
{
    /* 习惯学习简化版：基于时间窗口的例行提醒 */
    /* 完整实现：统计用户作息，检测偏离后提醒 */
    static bool s_routine_alerted = false;
    if (!s_routine_alerted && s_log_count > 0) {
        /* 示例：如果22:00-23:00常用灯，23:30后仍开着则提醒 */
        proactive_alert_t alert;
        snprintf(alert.rule_id, sizeof(alert.rule_id), "routine");
        snprintf(alert.message, sizeof(alert.message), "已经快十一点了，记得早点休息。");
        alert.priority = 5;
        proactive_push_alert(&alert);
        s_routine_alerted = true;
    }
}

static void check_morning(void)
{
    /* 晨报：需要 RTC/SNTP 时间，7:30 触发一次 */
    /* 演示：由网关/云端推送或本地时间触发 */
}

void proactive_tick(void)
{
    uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);

    for (int i = 0; i < RULE_COUNT; i++) {
        if (now - s_last_check[i] >= s_rules[i].interval_ms) {
            s_last_check[i] = now;
            if (strcmp(s_rules[i].rule_id, "env_alarm") == 0) {
                check_env_alarm();
            } else if (strcmp(s_rules[i].rule_id, "routine") == 0) {
                check_routine();
            } else if (strcmp(s_rules[i].rule_id, "morning") == 0) {
                check_morning();
            }
        }
    }
}

void proactive_push_alert(const proactive_alert_t *alert)
{
    if (!alert) {
        return;
    }
    /* 接入 xiaozhi-esp32 的 TTS 打断播放接口 */
    /* xiaozhi_say(alert->message);  // 由官方接口提供 */
    ESP_LOGI(TAG, "[ALERT p=%d] %s", alert->priority, alert->message);
}
