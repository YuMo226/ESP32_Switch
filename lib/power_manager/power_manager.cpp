#include "power_manager.h"
#include <WiFi.h>

// ESP-IDF 底层 API
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_pm.h"

void PowerManager::init() {
  // 禁用欠压检测器（开发阶段，供电不足时防止重启循环）
  REG_WRITE(RTC_CNTL_BROWN_OUT_REG, 0);

  // 设置 CPU 为 80MHz（比默认 160MHz 省电约一半）
  setCpuFreq(CPU_FREQ_MHZ);

#if DEBUG_SERIAL
  Serial.printf("[Power] CPU 频率: %d MHz\n", getCpuFrequencyMhz());
  Serial.flush();
#endif
}

void PowerManager::setCpuFreq(uint32_t mhz) {
  setCpuFrequencyMhz(mhz);

#if DEBUG_SERIAL
  Serial.printf("[Power] CPU 频率切换: %d MHz → %d MHz\n", getCpuFrequencyMhz(), mhz);
  Serial.flush();
#endif
}

void PowerManager::disableWiFi() {
  WiFi.disconnect(true);   // 断开连接并清除凭据
  WiFi.mode(WIFI_OFF);
#if DEBUG_SERIAL
  Serial.println("[Power] WiFi 已关闭");
  Serial.flush();
#endif
}

void PowerManager::disableBLE() {
  // BLE 关闭由 BleManager 内部处理（需要先释放 BLE 资源）
  // 这里仅作备用接口
}

bool PowerManager::initWiFiSafe() {
#if DEBUG_SERIAL
  Serial.println("[Power] 安全初始化 WiFi...");
  Serial.flush();
#endif

  // 保存当前 CPU 频率
  uint32_t originalFreq = getCpuFrequencyMhz();

  // 降低 CPU 频率以减少瞬间电流
  setCpuFrequencyMhz(40);

  // 延迟让电源稳定
  delay(100);

  // 尝试初始化 WiFi
  WiFi.mode(WIFI_STA);
  delay(50);

  bool success = (WiFi.status() != WL_NO_SHIELD);

  // 恢复 CPU 频率
  setCpuFrequencyMhz(originalFreq);

#if DEBUG_SERIAL
  if (success) {
    Serial.println("[Power] WiFi 初始化成功");
  } else {
    Serial.println("[Power] WiFi 初始化失败");
  }
  Serial.flush();
#endif

  return success;
}

void PowerManager::enterLowPowerMode() {
#if DEBUG_SERIAL
  Serial.println("[Power] 进入低功耗模式");
  Serial.flush();
#endif

  // 降低 CPU 频率
  setCpuFrequencyMhz(40);

  // 关闭 WiFi（如果不需要）
  disableWiFi();
}

void PowerManager::exitLowPowerMode() {
#if DEBUG_SERIAL
  Serial.println("[Power] 退出低功耗模式");
  Serial.flush();
#endif

  // 恢复 CPU 频率
  setCpuFrequencyMhz(CPU_FREQ_MHZ);
}
