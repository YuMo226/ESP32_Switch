#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
//  全局配置 — ESP32-C3 低功耗旋钮系统
// ============================================================

// ---------- 调试 ----------
#define DEBUG_SERIAL          1
#define SERIAL_BAUD           115200

// ---------- GPIO 引脚 ----------
#define PIN_ENC_A             5
#define PIN_ENC_B             6
#define PIN_BUTTON            4
#define PIN_BATTERY_ADC       0       // 电池电压 ADC 引脚（需确认硬件）

// ---------- 编码器 ----------
#define ENC_DEBOUNCE_US       200     // 旋转防抖（微秒）
#define BTN_DEBOUNCE_MS       50      // 按键防抖（毫秒）
#define PULSES_PER_DETENT     4       // 每个咔嗒的四倍频跳变数（20脉冲 EC11 = 4）
#define ENC_POS_MIN           0
#define ENC_POS_MAX           100

// ---------- 功耗与时序 ----------
#define CPU_FREQ_MHZ          80      // 默认 CPU 频率（比 160MHz 省电）
#define IDLE_TIMEOUT_MS       5000    // 空闲超时 → Deep Sleep（毫秒）
#define AGG_WINDOW_MS         100     // ESP-NOW 事件聚合窗口（毫秒）
#define BLE_IDLE_TIMEOUT_MS   60000   // BLE 空闲超时 → 断开（毫秒）
#define LONG_PRESS_MS         1000    // 长按阈值（毫秒）
#define DOUBLE_CLICK_MS       300     // 双击间隔阈值（毫秒）

// ---------- BLE ----------
#define BLE_DEVICE_NAME       "Smart_Knob_C3"

// ---------- ESP-NOW ----------
// TODO: 填入接收端实际 MAC 地址
#define RECEIVER_MAC          {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

// ---------- 电池 ----------
#define BATT_FULL_MV          4200    // 满电电压（mV）
#define BATT_EMPTY_MV         3300    // 空电电压（mV）
#define BATT_LOW_THRESHOLD    10      // 低电量告警阈值（%）

// ---------- ESP-NOW 数据包 ----------
#define PKT_DEVICE_ID        0x01    // 设备标识
#define PKT_SIZE              8       // 数据包大小（字节）

// ---------- 事件队列 ----------
#define EVENT_QUEUE_SIZE      32

#endif // CONFIG_H
