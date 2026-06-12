#include <Arduino.h>
#include "config.h"

// ---- 模块头文件 ----
#include "power_manager.h"
#include "storage_manager.h"
#include "config_manager.h"
#include "sleep_manager.h"
#include "encoder_manager.h"
#include "button_manager.h"
#include "event_manager.h"
#include "esp_now_manager.h"
#include "ble_manager.h"
#include "battery_manager.h"
#include "state_machine.h"

// ============================================================
//  ESP32-C3 低功耗旋钮 — 主程序
//
//  职责：
//  1. 初始化所有模块
//  2. 调度状态机
//
//  不做：
//  - 不处理业务逻辑（交给 StateMachine）
//  - 不直接操作无线（交给对应 Manager）
// ============================================================

// ---- 全局模块实例 ----
static PowerManager    power;
static StorageManager  storage;
static ConfigManager   config;
static SleepManager    sleepMgr;
static EncoderManager  encoder;
static ButtonManager   button;
static EventManager    events;
static EspNowManager   espNow;
static BleManager      ble;
static BatteryManager  battery;
static StateMachine    stateMachine;

// ---- 辅助函数：打印分隔线 ----
static void printSeparator() {
  Serial.println("========================================");
  Serial.flush();
}

// ---- 辅助函数：打印步骤日志 ----
static void printStep(const char* step) {
  Serial.printf("[BOOT] %s\n", step);
  Serial.flush();
}

// ---- 辅助函数：打印内存状态 ----
static void printMemoryStatus(const char* label) {
  Serial.printf("[MEM] %s - 可用堆: %d bytes\n", label, ESP.getFreeHeap());
  Serial.flush();
}

// ============================================================
//  setup — 初始化所有模块
// ============================================================

void setup() {
  // 1. 底层初始化
  power.init();               // CPU 80MHz + 禁用 brownout
  Serial.begin(SERIAL_BAUD);
  delay(300);  // 等待串口稳定

  printSeparator();
  Serial.println("  ESP32-C3 低功耗旋钮 v2.0");
  Serial.println("  状态机架构");
  printSeparator();

  printMemoryStatus("启动");
  printStep("step1 - power init done");

  // 2. 存储 + 配置
  storage.begin();
  config.begin(&storage);
  config.load();
  printStep("step2 - storage & config done");

  // 3. 检测唤醒原因
  WakeReason wake = sleepMgr.getWakeReason();
  Serial.printf("[Boot] 唤醒原因: %s\n",
    wake == WakeReason::GPIO ? "GPIO" : "上电/复位");
  Serial.flush();
  printStep("step3 - wake reason detected");

  // 4. 硬件模块初始化（低功耗模块，不影响电源）
  battery.begin();
  events.clear();
  encoder.begin(&events);
  button.begin(&events);
  printStep("step4 - hardware modules done");
  printMemoryStatus("硬件初始化后");

  // GPIO 唤醒时恢复编码器相位
  if (wake == WakeReason::GPIO) {
    encoder.restoreState(sleepMgr.getSavedEncoderState());
    printStep("step4.1 - encoder state restored");
  }

  // 5. 延迟初始化无线模块
  // 这是关键：让电源和系统完全稳定后再初始化 WiFi
  printStep("step5 - waiting for system stability...");
  delay(500);  // 额外延迟 500ms，让电源稳定
  printMemoryStatus("延迟后");

  // 初始化 ESP-NOW（内部会检查内存并使用底层 API）
  printStep("step5.1 - starting ESP-NOW initialization...");
  bool espNowOk = espNow.begin(config.get().receiverMAC);
  if (espNowOk) {
    printStep("step5.2 - ESP-NOW initialized successfully");
  } else {
    printStep("step5.2 - ESP-NOW initialization failed, continuing without it");
  }
  printMemoryStatus("ESP-NOW 初始化后");

  // 6. 状态机初始化（绑定所有模块）
  stateMachine.begin(&events,
                     &encoder, &button,
                     &espNow, &ble,
                     &battery, &power,
                     &sleepMgr, &config);
  printStep("step6 - state machine initialized");

  printSeparator();
  Serial.println("[Boot] 初始化完成");
  Serial.printf("[Boot] ESP-NOW 状态: %s\n", espNowOk ? "就绪" : "不可用");
  Serial.printf("[Boot] CPU 频率: %d MHz\n", getCpuFrequencyMhz());
  printMemoryStatus("最终");
  printSeparator();
}

// ============================================================
//  loop — 调度状态机
// ============================================================

void loop() {
  // 输入采集（ISR 标志 → 事件队列）
  encoder.update();
  button.update();

  // 状态机驱动（消费事件 → 状态转换 → 动作执行）
  stateMachine.update();
}
