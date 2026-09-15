# 小智终端二次开发模块 CMakeLists
# 合并进 xiaozhi-esp32 工程 main/ 目录时，将本文件内容并入官方 main/CMakeLists.txt 的 SRCS

idf_component_register(
    SRCS
        "main.c"
        "mcp_tools.c"
        "ble_gateway.c"
        "mqtt_bridge.c"
        "motion.c"
        "sensor_fusion.c"
        "proactive.c"
        "offline_cmd.c"
        "lcd_ui.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_wifi nvs_flash esp_netif mqtt bt
)
