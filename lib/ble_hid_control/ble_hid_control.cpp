#include "ble_hid_control.h"
#include "global_config.h"

// ============================================================
//  蓝牙 BLE HID 实现
//
//  依赖库: T-vK/ESP32-BLE-Keyboard
// ============================================================

#include <BleKeyboard.h>
#include <BLEDevice.h>
static BleKeyboard bleKeyboard(BLE_DEVICE_NAME, "Espressif", 100);

// ============================================================
//  初始化
// ============================================================

void BLEHIDControl::begin() {
#if BLE_ENABLED
  bleKeyboard.begin();

  // 配置 BLE 安全参数（HID over GATT 要求加密连接）
  BLESecurity* pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);     // 配对后绑定，重连无需再次配对
  pSecurity->setCapability(ESP_IO_CAP_NONE);              // 无输入无输出（Just Works 配对）
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  _initialized = true;

#if DEBUG_SERIAL
  Serial.print("[BLE] HID 设备 \"");
  Serial.print(BLE_DEVICE_NAME);
  Serial.println("\" 已启动，等待配对...");
#endif

#else
#if DEBUG_SERIAL
  Serial.println("[BLE] 已禁用（BLE_ENABLED=0）");
#endif
#endif
}

// ============================================================
//  主循环维护
// ============================================================

void BLEHIDControl::loop() {
  // BleKeyboard 底层 BLE 协议栈自动维护连接，无需显式调用
}

// ============================================================
//  连接状态
// ============================================================

bool BLEHIDControl::isConnected() const {
  return _initialized && bleKeyboard.isConnected();
}

// ============================================================
//  多媒体控制
// ============================================================

void BLEHIDControl::sendVolumeUp() {
  if (!isConnected()) return;

  bleKeyboard.write(KEY_MEDIA_VOLUME_UP);

#if DEBUG_SERIAL
  Serial.println("[BLE] 发送: 音量+");
#endif
}

void BLEHIDControl::sendVolumeDown() {
  if (!isConnected()) return;

  bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);

#if DEBUG_SERIAL
  Serial.println("[BLE] 发送: 音量-");
#endif
}

void BLEHIDControl::sendMute() {
  if (!isConnected()) return;

  bleKeyboard.write(KEY_MEDIA_MUTE);

#if DEBUG_SERIAL
  Serial.println("[BLE] 发送: 静音");
#endif
}
