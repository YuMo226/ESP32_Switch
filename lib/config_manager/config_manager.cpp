#include "config_manager.h"
#include "storage_manager.h"

static const char* NVS_KEY_CONFIG = "dev_cfg";

void ConfigManager::begin(StorageManager* storage) {
  _storage = storage;
  _setDefaults();
#if DEBUG_SERIAL
  Serial.println("[Config] 初始化完成");
#endif
}

void ConfigManager::load() {
  if (!_storage) return;

  if (_storage->load(NVS_KEY_CONFIG, &_config, sizeof(DeviceConfig))) {
#if DEBUG_SERIAL
    Serial.printf("[Config] 已加载: name=%s, mode=%s\n",
                  _config.deviceName,
                  _config.commMode == CommMode::ESP_NOW ? "ESP-NOW" : "BLE");
#endif
  } else {
    // NVS 中无数据，使用默认值并保存
    _setDefaults();
    save();
#if DEBUG_SERIAL
    Serial.println("[Config] 无已存配置，使用默认值");
#endif
  }
}

void ConfigManager::save() {
  if (!_storage) return;
  _storage->save(NVS_KEY_CONFIG, &_config, sizeof(DeviceConfig));
#if DEBUG_SERIAL
  Serial.println("[Config] 配置已保存");
#endif
}

void ConfigManager::reset() {
  _setDefaults();
  save();
#if DEBUG_SERIAL
  Serial.println("[Config] 已恢复默认配置");
#endif
}

DeviceConfig& ConfigManager::get() {
  return _config;
}

void ConfigManager::_setDefaults() {
  memset(&_config, 0, sizeof(DeviceConfig));
  strncpy(_config.deviceName, BLE_DEVICE_NAME, sizeof(_config.deviceName) - 1);

  uint8_t defaultMAC[6] = RECEIVER_MAC;
  memcpy(_config.receiverMAC, defaultMAC, 6);

  _config.commMode      = CommMode::ESP_NOW;
  _config.sensitivity   = 5;
  _config.sleepTimeoutMs = IDLE_TIMEOUT_MS;
  _config.bleTimeoutMs  = BLE_IDLE_TIMEOUT_MS;
}
