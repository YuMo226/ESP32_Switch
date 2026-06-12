#ifndef GLOBAL_CONFIG_H
#define GLOBAL_CONFIG_H

// ============================================================
//  全局配置 — 引脚、参数、调试开关
// ============================================================

// ---------- 调试开关 ----------
#define DEBUG_SERIAL  1           // 1 = 开启串口调试输出，0 = 关闭

// ---------- 串口配置 ----------
#define SERIAL_BAUD   115200

// ---------- EC11 旋钮引脚 ----------
#define PIN_ENC_A     5           // 编码器 A 相
#define PIN_ENC_B     6           // 编码器 B 相
#define PIN_BUTTON    4           // 编码器内置按键

// ---------- EC11 防抖参数 ----------
#define ENC_DEBOUNCE_US   200     // 旋转防抖（微秒）
#define BTN_DEBOUNCE_MS   50      // 按键防抖（毫秒）

// ---------- 蓝牙 BLE HID 配置 ----------
#define BLE_DEVICE_NAME   "Smart_Knob_C3"   // 电脑蓝牙搜索到的设备名
#define BLE_ENABLED       1                  // 1 = 启用 BLE，0 = 禁用

// ---------- ESP-NOW 配置 ----------
// TODO: 填入接收端的 MAC 地址
// 格式: {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
#define RECEIVER_MAC  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

// ---------- Wi-Fi 配置（IoT 远程控制用） ----------
#define WIFI_SSID     "your-ssid"
#define WIFI_PASSWORD "your-password"

// ---------- MQTT 配置（IoT 远程控制用） ----------
#define MQTT_SERVER   "broker.hivemq.com"
#define MQTT_PORT     1883
#define MQTT_TOPIC_CMD    "esp32/switch/cmd"
#define MQTT_TOPIC_STATUS "esp32/switch/status"

#endif // GLOBAL_CONFIG_H
