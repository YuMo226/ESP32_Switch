#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include <Arduino.h>
#include "config.h"

// ============================================================
//  电池管理器 — ADC 电压检测
//
//  ESP32-C3 ADC 范围: 0-3.3V (12-bit: 0-4095)
//  如果电池经过分压电阻，需在 config.h 中配置分压比
// ============================================================

class BatteryManager {
public:
  void begin();

  /**
   * 读取电池电量百分比 (0-100)
   */
  uint8_t readPercent();

  /**
   * 读取电池电压 (mV)
   */
  uint16_t readMilliVolts();

  /**
   * 是否低电量
   */
  bool isLow() const;

  /**
   * 获取上次读取的电量百分比（不触发新的 ADC 读取）
   */
  uint8_t getLastPercent() const;

private:
  uint8_t _lastPercent = 100;
};

#endif // BATTERY_MANAGER_H
