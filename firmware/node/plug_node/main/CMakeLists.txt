idf_component_register(
    SRCS "main.c" "../../ble_node/ble_node.c"
    INCLUDE_DIRS "." "../../ble_node"
    REQUIRES driver nvs_flash bt
)
