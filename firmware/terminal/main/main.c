/*
 * main.c - 小智终端二次开发入口
 * 小智·智家 XiaoZhi SmartHome
 *
 * 集成说明：
 *  - 本文件作为 xiaozhi-esp32 工程的增量入口模块
 *  - 官方主流程（唤醒/ASR/LLM/TTS）由官方 main 驱动
 *  - 本模块负责：外设初始化、MCP 工具注册、主动引擎 tick、
 *    离线命令词拦截、状态 UI 驱动
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mcp_tools.h"
#include "ble_gateway.h"
#include "mqtt_bridge.h"
#include "motion.h"
#include "sensor_fusion.h"
#include "proactive.h"
#include "offline_cmd.h"
#include "lcd_ui.h"

static const char *TAG = "xiaozhi_smarthome";

/* 周期任务句柄 */
static TaskHandle_t s_house_task = NULL;

/* 主动引擎 tick + UI 刷新（1s 周期） */
static void house_task(void *arg)
{
    for (;;) {
        proactive_tick();
        lcd_ui_tick();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* ---------- 初始化入口（由官方 main 调用） ---------- */
void xiaozhi_smarthome_init(void)
{
    ESP_LOGI(TAG, "=== XiaoZhi SmartHome module init ===");

    /* 外设 */
    lcd_ui_init();
    motion_init();   /* 移动底盘 */

    /* 通信：WiFi 联网(MQTT) + 蓝牙 BLE 双通道（参考米家模式） */
    mqtt_bridge_init();
    ble_gateway_init();

    /* 智能层 */
    sensor_fusion_init_placeholder();
    proactive_init();
    offline_cmd_init();

    /* MCP 工具注册（供大模型调用） */
    mcp_tools_register_all();

    /* 周期任务 */
    xTaskCreate(house_task, "house", 4096, NULL, 5, &s_house_task);

    /* 启动动画 */
    lcd_ui_set_state(UI_STATE_IDLE);
    lcd_ui_show_face("happy");
    lcd_ui_show_text("小智·智家 已就绪");

    ESP_LOGI(TAG, "XiaoZhi SmartHome ready. Say: 小智小智");
}

/* ---------- 语音结果钩子（供官方 ASR 回调调用） ----------
 * 1. 先尝试离线命令词（断网/在线都可）
 * 2. 在线场景交给大模型 + MCP 工具链
 */
bool on_voice_result_hook(const char *text)
{
    if (!text) {
        return false;
    }

    /* 离线命令词引擎（本地兜底） */
    if (offline_cmd_process(text)) {
        lcd_ui_set_state(UI_STATE_EXECUTE);
        return true; /* 已本地处理，不再上云 */
    }

    /* 在线：返回 false 表示继续走官方 LLM + MCP 流程 */
    return false;
}

/* ============ 兼容性封装 ============ */
void sensor_fusion_init_placeholder(void)
{
    /* sensor_fusion 无独立初始化，占位保持 API 对称 */
}
