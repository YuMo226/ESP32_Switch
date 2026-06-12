#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "config.h"

// ============================================================
//  电源管理器 — CPU 频率 + 外设电源控制
// ============================================================

class PowerManager {
public:
  /**
   * 初始化：设置 CPU 为 80MHz，禁用 brownout
   */
  void init();

  /**
   * 动态调整 CPU 频率
   */
  void setCpuFreq(uint32_t mhz);

  /**
   * 关闭 WiFi 射频（ESP-NOW 发送完毕后调用）
   */
  void disableWiFi();

  /**
   * 关闭 BLE 射频
   */
  void disableBLE();

  /**
   * 安全初始化 WiFi（降低 CPU 频率，添加延迟，减少瞬间电流）
   * @return true 如果初始化成功
   */
  bool initWiFiSafe();

  /**
   * 进入低功耗模式（降低 CPU 频率，关闭无线）
   */
  void enterLowPowerMode();

  /**
   * 退出低功耗模式（恢复 CPU 频率）
   */
  void exitLowPowerMode();
};

#endif // POWER_MANAGER_H
