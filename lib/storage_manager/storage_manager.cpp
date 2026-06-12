#include "storage_manager.h"
#include <nvs_flash.h>
#include <nvs.h>

void StorageManager::begin() {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS 分区损坏或版本不匹配，擦除并重新初始化
    nvs_flash_erase();
    nvs_flash_init();
  }

  // 打开 NVS 命名空间
  nvs_handle_t handle;
  err = nvs_open("knob", NVS_READWRITE, &handle);
  if (err == ESP_OK) {
    _handle = (void*)(uintptr_t)handle;
    _initialized = true;
  }

#if DEBUG_SERIAL
  Serial.printf("[Storage] NVS 初始化%s\n", _initialized ? "完成" : "失败");
#endif
}

bool StorageManager::save(const char* key, const void* data, size_t len) {
  if (!_initialized) return false;
  nvs_handle_t handle = (nvs_handle_t)(uintptr_t)_handle;
  esp_err_t err = nvs_set_blob(handle, key, data, len);
  if (err == ESP_OK) {
    nvs_commit(handle);
  }
  return err == ESP_OK;
}

bool StorageManager::load(const char* key, void* data, size_t len) {
  if (!_initialized) return false;
  nvs_handle_t handle = (nvs_handle_t)(uintptr_t)_handle;
  return nvs_get_blob(handle, key, data, &len) == ESP_OK;
}

bool StorageManager::saveString(const char* key, const char* value) {
  if (!_initialized) return false;
  nvs_handle_t handle = (nvs_handle_t)(uintptr_t)_handle;
  esp_err_t err = nvs_set_str(handle, key, value);
  if (err == ESP_OK) {
    nvs_commit(handle);
  }
  return err == ESP_OK;
}

bool StorageManager::loadString(const char* key, char* buf, size_t maxLen) {
  if (!_initialized) return false;
  nvs_handle_t handle = (nvs_handle_t)(uintptr_t)_handle;
  return nvs_get_str(handle, key, buf, &maxLen) == ESP_OK;
}

bool StorageManager::saveInt(const char* key, int32_t value) {
  if (!_initialized) return false;
  nvs_handle_t handle = (nvs_handle_t)(uintptr_t)_handle;
  esp_err_t err = nvs_set_i32(handle, key, value);
  if (err == ESP_OK) {
    nvs_commit(handle);
  }
  return err == ESP_OK;
}

bool StorageManager::loadInt(const char* key, int32_t* value) {
  if (!_initialized) return false;
  nvs_handle_t handle = (nvs_handle_t)(uintptr_t)_handle;
  return nvs_get_i32(handle, key, value) == ESP_OK;
}

void StorageManager::erase(const char* key) {
  if (!_initialized) return;
  nvs_handle_t handle = (nvs_handle_t)(uintptr_t)_handle;
  nvs_erase_key(handle, key);
  nvs_commit(handle);
}

void StorageManager::eraseAll() {
  if (!_initialized) return;
  nvs_handle_t handle = (nvs_handle_t)(uintptr_t)_handle;
  nvs_erase_all(handle);
  nvs_commit(handle);
}
