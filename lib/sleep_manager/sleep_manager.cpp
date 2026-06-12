#include "sleep_manager.h"
#include <esp_sleep.h>

// RTC 内存变量定义
RTC_DATA_ATTR uint8_t  SleepManager::_savedEncState = 0;
RTC_DATA_ATTR uint32_t SleepManager::_sleepCount    = 0;

WakeReason SleepManager::getWakeReason() {
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

  switch (cause) {
    case ESP_SLEEP_WAKEUP_GPIO:
      return WakeReason::GPIO;
    case ESP_SLEEP_WAKEUP_UNDEFINED:
    default:
      return WakeReason::RESET;
  }
}

void SleepManager::prepareSleep(uint8_t encoderState) {
  // 保存编码器相位到 RTC 内存
  _savedEncState = encoderState;
  _sleepCount++;

#if DEBUG_SERIAL
  Serial.printf("[Sleep] 准备进入 Deep Sleep (第 %d 次)\n", _sleepCount);
  Serial.printf("[Sleep] 保存编码器相位: 0x%02X\n", encoderState);
#endif

  // 配置 GPIO 唤醒
  // 编码器引脚内部上拉（空闲 HIGH），旋转变 LOW
  // 按键引脚内部上拉（空闲 HIGH），按下变 LOW
  // 所以用低电平唤醒
  uint64_t pinMask = (1ULL << PIN_ENC_A) | (1ULL << PIN_ENC_B) | (1ULL << PIN_BUTTON);
  esp_deep_sleep_enable_gpio_wakeup(pinMask, ESP_GPIO_WAKEUP_GPIO_LOW);
}

void SleepManager::enterDeepSleep() {
#if DEBUG_SERIAL
  Serial.println("[Sleep] 进入 Deep Sleep...");
  Serial.flush();   // 确保串口输出完毕
  delay(10);
#endif

  esp_deep_sleep_start();
  // 不会执行到这里
}

uint8_t SleepManager::getSavedEncoderState() const {
  return _savedEncState;
}
