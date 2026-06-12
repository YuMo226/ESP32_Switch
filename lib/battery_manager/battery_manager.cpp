#include "battery_manager.h"

void BatteryManager::begin() {
  analogReadResolution(12);    // 12-bit ADC (0-4095)
  analogSetAttenuation(ADC_11db);  // 满量程 ~3.3V

  // 多次读取稳定 ADC
  for (int i = 0; i < 5; i++) {
    analogRead(PIN_BATTERY_ADC);
    delay(10);
  }

  // 初始化时读取一次
  uint8_t initialPercent = readPercent();

#if DEBUG_SERIAL
  Serial.printf("[Battery] 初始化完成: %d%% (%d mV)\n",
                initialPercent, readMilliVolts());
  Serial.flush();
#endif
}

uint16_t BatteryManager::readMilliVolts() {
  // 多次采样取平均，减少噪声
  uint32_t sum = 0;
  for (int i = 0; i < 16; i++) {
    sum += analogRead(PIN_BATTERY_ADC);
    delayMicroseconds(100);
  }
  uint16_t raw = sum / 16;

  // ADC 值转换为电压 (mV)
  // ESP32-C3 ADC: 0-4095 对应 0-3300mV（11dB 衰减）
  uint16_t voltage_mv = (uint32_t)raw * 3300 / 4095;

  // TODO: 如果有分压电阻，需乘以分压比
  // voltage_mv = voltage_mv * 2;  // 例如 1:1 分压

  return voltage_mv;
}

uint8_t BatteryManager::readPercent() {
  uint16_t mv = readMilliVolts();

  // 线性映射到百分比
  if (mv >= BATT_FULL_MV) {
    _lastPercent = 100;
  } else if (mv <= BATT_EMPTY_MV) {
    _lastPercent = 0;
  } else {
    _lastPercent = (uint32_t)(mv - BATT_EMPTY_MV) * 100 / (BATT_FULL_MV - BATT_EMPTY_MV);
  }

  // 防止抖动：只在电量变化超过 5% 时才更新
  static uint8_t lastReportedPercent = 255;
  if (lastReportedPercent == 255 || abs((int)_lastPercent - (int)lastReportedPercent) >= 5) {
#if DEBUG_SERIAL
    Serial.printf("[Battery] 电量: %d%% (%d mV)\n", _lastPercent, mv);
    Serial.flush();
#endif
    lastReportedPercent = _lastPercent;
  }

  return _lastPercent;
}

bool BatteryManager::isLow() const {
  return _lastPercent <= BATT_LOW_THRESHOLD;
}

uint8_t BatteryManager::getLastPercent() const {
  return _lastPercent;
}
