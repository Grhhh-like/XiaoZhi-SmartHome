idf_component_register(
    SRCS "main.c" "../../espnow_node/espnow_node.c"
    INCLUDE_DIRS "." "../../espnow_node"
    REQUIRES driver esp_wifi esp_now nvs_flash
)
