/*
 * lcd_ui.h - 圆屏表情 UI 模块
 * 小智·智家 XiaoZhi SmartHome
 * 显示小智的听/想/说/执行 四种状态动画与表情，提升互动性
 */
#ifndef LCD_UI_H
#define LCD_UI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UI_STATE_IDLE = 0,   /* 待机 */
    UI_STATE_LISTEN,     /* 聆听 */
    UI_STATE_THINK,      /* 思考 */
    UI_STATE_SPEAK,      /* 说话 */
    UI_STATE_EXECUTE,    /* 执行 */
    UI_STATE_ALERT,      /* 提醒 */
} ui_state_t;

/* 初始化屏幕与 UI */
void lcd_ui_init(void);

/* 切换状态（播放对应动画） */
void lcd_ui_set_state(ui_state_t state);

/* 显示表情（自定义：happy/sad/sleep/...） */
void lcd_ui_show_face(const char *face);

/* 显示文本（滚动/静态） */
void lcd_ui_show_text(const char *text);

/* 周期性刷新动画帧 */
void lcd_ui_tick(void);

#ifdef __cplusplus
}
#endif

#endif /* LCD_UI_H */
