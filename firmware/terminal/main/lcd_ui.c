/*
 * lcd_ui.c - 圆屏表情 UI 模块实现
 * 小智·智家 XiaoZhi SmartHome
 * 基于 LVGL + GC9A01 圆屏，绘制状态动画
 */
#include <string.h>
#include "esp_log.h"
#include "lcd_ui.h"

static const char *TAG = "lcd_ui";

static ui_state_t s_state = UI_STATE_IDLE;
static const char *s_face = "default";
static char s_text[64] = {0};
static uint32_t s_frame = 0;

/* 状态对应的动画帧数 */
static const int s_state_frames[UI_STATE_ALERT + 1] = {
    1,  /* IDLE: 静态眨眼 */
    4,  /* LISTEN: 波纹扩散 */
    3,  /* THINK: 转圈 */
    3,  /* SPEAK: 声波动画 */
    3,  /* EXECUTE: 打勾/齿轮 */
    2,  /* ALERT: 闪烁感叹号 */
};

void lcd_ui_init(void)
{
    /* 与 xiaozhi-esp32 集成时，此处初始化官方 LCD/LVGL 层 */
    ESP_LOGI(TAG, "LCD UI init (GC9A01 240x240 round)");
}

void lcd_ui_set_state(ui_state_t state)
{
    if (state > UI_STATE_ALERT) {
        return;
    }
    s_state = state;
    s_frame = 0;
    ESP_LOGD(TAG, "ui state -> %d", state);
}

void lcd_ui_show_face(const char *face)
{
    if (!face) {
        return;
    }
    snprintf(s_face, 16, "%s", face);
    ESP_LOGD(TAG, "face -> %s", face);
}

void lcd_ui_show_text(const char *text)
{
    if (!text) {
        return;
    }
    snprintf(s_text, sizeof(s_text), "%s", text);
    ESP_LOGD(TAG, "text -> %s", text);
}

void lcd_ui_tick(void)
{
    s_frame++;
    int frames = s_state_frames[s_state];
    uint32_t frame_idx = s_frame % frames;

    /* 说明：此处为渲染占位。
     * 与 LVGL 集成时，根据 (s_state, frame_idx) 绘制：
     *  - IDLE: 眨眼
     *  - LISTEN: 波纹
     *  - THINK: 转圈
     *  - SPEAK: 声波
     *  - EXECUTE: 打勾
     *  - ALERT: 感叹号
     */
    ESP_LOGV(TAG, "frame %d/%d state=%d", frame_idx, frames, s_state);
}
