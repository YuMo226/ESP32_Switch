#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>

// ============================================================
//  存储管理器 — NVS 持久存储
//
//  封装 ESP-IDF NVS (Non-Volatile Storage) API。
//  用于保存设备配置、配对信息、用户设置等。
//  数据在 Deep Sleep 和断电后保持。
// ============================================================

class StorageManager {
public:
  /**
   * 初始化 NVS 分区
   */
  void begin();

  /**
   * 保存二进制数据
   */
  bool save(const char* key, const void* data, size_t len);

  /**
   * 读取二进制数据
   */
  bool load(const char* key, void* data, size_t len);

  /**
   * 保存字符串
   */
  bool saveString(const char* key, const char* value);

  /**
   * 读取字符串
   */
  bool loadString(const char* key, char* buf, size_t maxLen);

  /**
   * 保存整数
   */
  bool saveInt(const char* key, int32_t value);

  /**
   * 读取整数
   */
  bool loadInt(const char* key, int32_t* value);

  /**
   * 删除指定键
   */
  void erase(const char* key);

  /**
   * 清除所有数据
   */
  void eraseAll();

private:
  bool _initialized = false;
  void* _handle = nullptr;   // nvs_handle_t
};

#endif // STORAGE_MANAGER_H
