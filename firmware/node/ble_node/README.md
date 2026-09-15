# 智能节点公共模块（BLE 从机）

> 自制智能节点以 **BLE（蓝牙低功耗）** 方式与终端组网，参考米家智能家居模式。
> 每个节点基于 ESP32-C3，作为 BLE GATT Server（外设），广播名 `XZ-<type>-<id>`，
> 终端（BLE Central）扫描连接后通过特征值下发控制、接收状态上报。

## 协议约定

| 项 | 值 |
|---|---|
| 广播名 | `XZ-<type>-<id>`（如 `XZ-01-00000001`） |
| 服务 UUID | `0000XZ00-0000-1000-8000-00805F9B34FB` |
| 控制特征值 | `0000XZ01-...`（Write，接收 `ble_node_ctrl_t`） |
| 状态特征值 | `0000XZ02-...`（Notify，状态/传感器上报） |

控制帧：

```c
typedef struct {
    char    device[24];   /* 节点ID */
    char    cmd[16];      /* on/off/set_brightness/open/close/stop/set_position/query */
    int32_t value;        /* 数值参数 */
} ble_node_ctrl_t;
```

## 使用

各节点工程 `main/CMakeLists.txt` 引用本模块：

```cmake
idf_component_register(
    SRCS "main.c" "../../ble_node/ble_node.c"
    INCLUDE_DIRS "." "../../ble_node"
    REQUIRES driver nvs_flash bt
)
```

启用 NimBLE：`idf.py menuconfig` → `Component config → Bluetooth → Bluedroid/NimBLE`（ESP32-C3 建议 NimBLE）。
