#ifndef SLEEP_MANAGER_H
#define SLEEP_MANAGER_H

#include <Arduino.h>
#include "config.h"

// ============================================================
//  睡眠管理器 — Deep Sleep + GPIO 唤醒
//
//  ESP32-C3 Deep Sleep 功耗: 5~20μA
//  唤醒源: GPIO 引脚电平变化（编码器 A/B/按键）
// ============================================================

enum class WakeReason : uint8_t {
  RESET           = 0,    // 上电/复位
  GPIO            = 1,    // GPIO 唤醒
  UNKNOWN         = 2,
};

class SleepManager {
public:
  /**
   * 读取唤醒原因（在 setup 最开始调用）
   */
  WakeReason getWakeReason();

  /**
   * 配置 GPIO 唤醒 + 保存状态
   * @param encoderState  编码器当前相位（保存到 RTC 内存）
   */
  void prepareSleep(uint8_t encoderState);

  /**
   * 进入 Deep Sleep（不返回）
   */
  void enterDeepSleep();

  /**
   * 获取 RTC 内存中保存的编码器相位
   */
  uint8_t getSavedEncoderState() const;

private:
  // RTC 内存 — Deep Sleep 期间保持数据
  // 注意: 必须是全局 static 或 RTC_DATA_ATTR
  static RTC_DATA_ATTR uint8_t _savedEncState;
  static RTC_DATA_ATTR uint32_t _sleepCount;
};

#endif // SLEEP_MANAGER_H
