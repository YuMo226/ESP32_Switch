#include "ble_manager.h"
#include <BleKeyboard.h>
#include <BLEDevice.h>

static BleKeyboard* pBleKb = nullptr;
static bool bleStackInitialized = false;

void BleManager::activate() {
  if (_initialized) return;

#if DEBUG_SERIAL
  Serial.printf("[BLE] 正在启动 BLE HID \"%s\" ...\n", BLE_DEVICE_NAME);
  Serial.flush();
#endif

  // 只创建一次 BleKeyboard 对象，之后复用
  if (!pBleKb) {
    pBleKb = new BleKeyboard(BLE_DEVICE_NAME, "Espressif", 100);
    pBleKb->begin();

    // 配对安全参数（只设置一次）
    if (!bleStackInitialized) {
      BLESecurity* pSec = new BLESecurity();
      pSec->setAuthenticationMode(ESP_LE_AUTH_BOND);
      pSec->setCapability(ESP_IO_CAP_NONE);
      pSec->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
      bleStackInitialized = true;
    }

#if DEBUG_SERIAL
    Serial.println("[BLE] HID 已启动，等待配对...");
#endif
  } else {
    // 复用已有的 BleKeyboard，重新开始广告
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    if (pAdvertising) {
      pAdvertising->start();
    }

#if DEBUG_SERIAL
    Serial.println("[BLE] HID 已重新启动，等待配对...");
#endif
  }

  _initialized = true;
  _lastActive = millis();

#if DEBUG_SERIAL
  Serial.flush();
#endif
}

void BleManager::deactivate() {
  if (!_initialized) return;

#if DEBUG_SERIAL
  Serial.println("[BLE] 暂停 BLE 广告...");
  Serial.flush();
#endif

  // 只停止广告，不释放 GATT handles
  // 这样可以复用 handles，避免 "no free handle blocks" 错误
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  if (pAdvertising) {
    pAdvertising->stop();
  }

  _initialized = false;

#if DEBUG_SERIAL
  Serial.println("[BLE] 已暂停");
  Serial.flush();
#endif
}

bool BleManager::isActive() const {
  return _initialized;
}

bool BleManager::isConnected() const {
  return _initialized && pBleKb && pBleKb->isConnected();
}

bool BleManager::isIdle() const {
  if (!_initialized) return false;
  return (millis() - _lastActive) >= BLE_IDLE_TIMEOUT_MS;
}

void BleManager::touch() {
  _lastActive = millis();
}

void BleManager::sendVolumeUp() {
  if (!_initialized || !pBleKb) return;
  pBleKb->write(KEY_MEDIA_VOLUME_UP);
  touch();
#if DEBUG_SERIAL
  Serial.println("[BLE] 音量+");
#endif
}

void BleManager::sendVolumeDown() {
  if (!_initialized || !pBleKb) return;
  pBleKb->write(KEY_MEDIA_VOLUME_DOWN);
  touch();
#if DEBUG_SERIAL
  Serial.println("[BLE] 音量-");
#endif
}

void BleManager::sendMute() {
  if (!_initialized || !pBleKb) return;
  pBleKb->write(KEY_MEDIA_MUTE);
  touch();
#if DEBUG_SERIAL
  Serial.println("[BLE] 静音");
#endif
}
