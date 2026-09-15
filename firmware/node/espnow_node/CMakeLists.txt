# 小智·智家 智能节点公共模块
# 各节点 main/CMakeLists.txt 引用：REQUIRES driver esp_wifi esp_now nvs_flash

idf_component_register(
    SRCS
        "espnow_node.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_wifi esp_now nvs_flash
)
