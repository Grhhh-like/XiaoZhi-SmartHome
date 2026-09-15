/*
 * mcp_tools.h - MCP 智能家居工具集定义
 * 小智·智家 XiaoZhi SmartHome
 *
 * 为云端大模型注册可调用的"工具"(MCP协议)，实现：
 * 语音指令 → 意图理解 → 工具调用 → 硬件执行 → 状态回读确认
 */
#ifndef MCP_TOOLS_H
#define MCP_TOOLS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------- 工具结果结构 ---------- */
typedef struct {
    bool     success;      /* 执行是否成功 */
    char     message[256]; /* 回读确认文本，用于 TTS 播报 */
    int32_t  value;        /* 数值型状态(如亮度/温度) */
    char     extra[128];   /* 附加信息(如房间/设备名) */
} mcp_result_t;

/* ---------- 工具枚举 ---------- */
typedef enum {
    MCP_TOOL_SMART_HOME_CONTROL = 0, /* 通用设备控制: device, cmd, value */
    MCP_TOOL_IR_SEND,                /* 红外: brand, device, cmd */
    MCP_TOOL_RF_SEND,                /* 433: remote_id, cmd */
    MCP_TOOL_QUERY_SENSOR,           /* 查询传感器: node_id, type */
    MCP_TOOL_NAS_QUERY,              /* NAS知识中枢: scope, query */
    MCP_TOOL_SCENE_EXECUTE,          /* 场景联动: scene */
    MCP_TOOL_MAX
} mcp_tool_id_t;

/* ---------- 工具调用参数 ---------- */
typedef struct {
    char device[32];   /* 设备名: living_room_light / bedroom_ac / ... */
    char cmd[32];      /* 命令: on / off / set_brightness / set_temp / ... */
    int32_t value;     /* 参数值: 亮度0-100 / 温度16-30 / 百分比 ... */
    char room[32];     /* 房间: living_room / bedroom / kitchen ... */
} mcp_smart_home_args_t;

typedef struct {
    char brand[16];    /* 品牌: gree / media / haier ... */
    char device[16];   /* 设备类型: ac / tv / fan */
    char cmd[16];      /* 命令: power_on / temp_26 / mode_cool ... */
} mcp_ir_args_t;

typedef struct {
    int  remote_id;    /* 433 插座遥控ID */
    char cmd[8];       /* on / off */
} mcp_rf_args_t;

typedef struct {
    char node_id[16];  /* 传感器节点ID: sensor_living */
    char type[16];     /* 类型: temp / humi / air / light */
} mcp_sensor_args_t;

typedef struct {
    char scope[16];    /* NAS范围: calendar / docs / todo / files */
    char query[128];   /* 查询内容 */
} mcp_nas_args_t;

typedef struct {
    char scene[32];    /* 场景名: sleep / home / away / movie ... */
} mcp_scene_args_t;

/* ---------- API ---------- */

/* 注册全部 MCP 工具到 xiaozhi-esp32 工具表 */
void mcp_tools_register_all(void);

/* 各工具实现（可被 MCP 调度器直接调用） */
mcp_result_t mcp_smart_home_control(const mcp_smart_home_args_t *args);
mcp_result_t mcp_ir_send(const mcp_ir_args_t *args);
mcp_result_t mcp_rf_send(const mcp_rf_args_t *args);
mcp_result_t mcp_query_sensor(const mcp_sensor_args_t *args);
mcp_result_t mcp_nas_query(const mcp_nas_args_t *args);
mcp_result_t mcp_scene_execute(const mcp_scene_args_t *args);

/* 工具 schema 描述（发给大模型，用于意图理解与参数抽取） */
const char *mcp_tools_get_schema_json(void);

#ifdef __cplusplus
}
#endif

#endif /* MCP_TOOLS_H */
