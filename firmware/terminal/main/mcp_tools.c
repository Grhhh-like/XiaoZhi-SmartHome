/*
 * mcp_tools.c - MCP 智能家居工具集实现
 * 小智·智家 XiaoZhi SmartHome
 *
 * 各工具实现核心调度逻辑（参考米家智能家居模式，WiFi+BLE 双通道）：
 *  - smart_home_control: 按设备类型路由到 WiFi联网(MQTT/HTTP) 或 蓝牙BLE 通道
 *  - query_sensor: BLE 回读传感器节点
 *  - nas_query: 通过局域网调用 NAS Bridge 服务(HTTP)
 *  - scene_execute: 场景原子动作组合执行
 */
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "mcp_tools.h"
#include "ble_gateway.h"
#include "mqtt_bridge.h"
#include "sensor_fusion.h"
#include "proactive.h"

static const char *TAG = "mcp_tools";

/* ============ 工具 schema（发送给大模型） ============ */
static const char *TOOLS_SCHEMA =
    "["
    "{\"name\":\"smart_home_control\","
    " \"desc\":\"控制智能家居设备(灯光/插座/窗帘/空调等)，WiFi联网+蓝牙BLE双通道\","
    " \"params\":{\"device\":\"设备名,如living_room_light\","
    "            \"cmd\":\"命令,如on/off/set_brightness/set_temp\","
    "            \"value\":\"数值,亮度0-100/温度16-30\","
    "            \"room\":\"房间,如living_room\"}},"
    "{\"name\":\"query_sensor\","
    " \"desc\":\"查询BLE环境传感器节点数据\","
    " \"params\":{\"node_id\":\"如sensor_living\",\"type\":\"temp/humi/air/light\"}},"
    "{\"name\":\"nas_query\","
    " \"desc\":\"查询NAS知识中枢(日程/文档/待办)\","
    " \"params\":{\"scope\":\"calendar/docs/todo/files\",\"query\":\"查询内容\"}},"
    "{\"name\":\"scene_execute\","
    " \"desc\":\"执行场景联动\","
    " \"params\":{\"scene\":\"sleep/home/away/movie\"}}"
    "]";

const char *mcp_tools_get_schema_json(void)
{
    return TOOLS_SCHEMA;
}

/* ============ smart_home_control: 通用设备控制 ============ */
mcp_result_t mcp_smart_home_control(const mcp_smart_home_args_t *args)
{
    mcp_result_t res = {0};

    if (!args || !args->device[0]) {
        res.success = false;
        snprintf(res.message, sizeof(res.message), "设备名不能为空");
        return res;
    }

    /* 按设备名路由到对应通道（参考米家：WiFi 主通道 + BLE 本地直连） */
    if (strstr(args->device, "light") || strstr(args->device, "lamp")) {
        /* 灯光 → BLE 灯节点（本地直连） */
        ble_light_ctrl(args->device, args->cmd, args->value);
        res.success = true;
        snprintf(res.message, sizeof(res.message),
                 "灯已%s%s", args->cmd, args->value ? "，亮度已调整" : "");
        res.value = args->value;
    } else if (strstr(args->device, "curtain") || strstr(args->device, "curtain_motor")) {
        /* 窗帘 → BLE 窗帘节点 */
        ble_curtain_ctrl(args->device, args->cmd, args->value);
        res.success = true;
        snprintf(res.message, sizeof(res.message), "窗帘已执行%s", args->cmd);
    } else if (strstr(args->device, "plug") || strstr(args->device, "socket")) {
        /* 插座 → BLE 插座节点 */
        ble_plug_ctrl(args->device, args->cmd);
        res.success = true;
        snprintf(res.message, sizeof(res.message), "插座已%s", args->cmd);
    } else if (strstr(args->device, "ac") || strstr(args->device, "air_cond")) {
        /* 空调 → WiFi 联网通道（米家/Home Assistant 生态，HTTP/MQTT） */
        mqtt_publish_device(args->device, args->cmd, args->value);
        res.success = true;
        snprintf(res.message, sizeof(res.message), "空调指令已发送（WiFi联网）");
        res.value = args->value;
    } else {
        /* 兜底：WiFi 联网通用通道（米家/HA 生态） */
        mqtt_publish_device(args->device, args->cmd, args->value);
        res.success = true;
        snprintf(res.message, sizeof(res.message), "设备 %s 指令已发送", args->device);
    }

    /* 记录主动引擎交互日志（供习惯学习） */
    proactive_log_interaction(args->device, args->cmd, res.success);
    return res;
}

/* ============ query_sensor: 传感器查询 ============ */
mcp_result_t mcp_query_sensor(const mcp_sensor_args_t *args)
{
    mcp_result_t res = {0};
    if (!args) {
        res.success = false;
        snprintf(res.message, sizeof(res.message), "传感器参数为空");
        return res;
    }

    float value = 0;
    int ok = ble_sensor_read(args->node_id, args->type, &value);
    if (ok == 0) {
        res.success = true;
        res.value = (int32_t)(value * 10); /* 保留一位小数 */
        const char *unit = sensor_fusion_unit(args->type);
        snprintf(res.message, sizeof(res.message), "%s 当前 %.1f%s",
                 args->node_id, value, unit);
    } else {
        res.success = false;
        snprintf(res.message, sizeof(res.message), "传感器 %s 无响应", args->node_id);
    }
    return res;
}

/* ============ nas_query: NAS 知识中枢 ============ */
mcp_result_t mcp_nas_query(const mcp_nas_args_t *args)
{
    mcp_result_t res = {0};
    if (!args) {
        res.success = false;
        snprintf(res.message, sizeof(res.message), "NAS查询参数为空");
        return res;
    }

    /* 通过局域网 HTTP 调用 NAS Bridge 服务（由 nas_bridge/bridge.py 提供） */
    int ok = nas_bridge_query(args->scope, args->query,
                              res.message, sizeof(res.message));
    res.success = (ok == 0);
    return res;
}

/* ============ scene_execute: 场景联动 ============ */
mcp_result_t mcp_scene_execute(const mcp_scene_args_t *args)
{
    mcp_result_t res = {0};
    if (!args) {
        res.success = false;
        snprintf(res.message, sizeof(res.message), "场景参数为空");
        return res;
    }

    int ok = proactive_execute_scene(args->scene);
    res.success = (ok == 0);
    if (res.success) {
        snprintf(res.message, sizeof(res.message), "%s场景已执行", args->scene);
    } else {
        snprintf(res.message, sizeof(res.message), "未找到场景 %s", args->scene);
    }
    return res;
}

/* ============ 注册入口 ============ */
void mcp_tools_register_all(void)
{
    /* 说明：此处为模块注册点。
     * 与 xiaozhi-esp32 集成时，将各工具函数指针注册进
     * 官方 MCP 工具表（参考官方 mcp_usage 文档）。
     */
    ESP_LOGI(TAG, "MCP tools registered: %d tools", MCP_TOOL_MAX);
    ESP_LOGI(TAG, "schema: %s", TOOLS_SCHEMA);
}
