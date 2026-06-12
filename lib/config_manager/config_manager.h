#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include "config.h"

class StorageManager;   // 前向声明

// ============================================================
//  配置管理器 — 设备配置持久化
//
//  管理设备名称、通信模式、配对信息、灵敏度等。
//  配置保存在 NVS 中，断电/Deep Sleep 后保持。
// ============================================================

// 通信模式
enum class CommMode : uint8_t {
  ESP_NOW   = 0,
  BLE       = 1,
};

// 设备配置结构体
struct DeviceConfig {
  char      deviceName[32];       // BLE 设备名
  uint8_t   receiverMAC[6];       // ESP-NOW 接收端 MAC
  CommMode  commMode;             // 当前通信模式
  uint8_t   sensitivity;          // 旋钮灵敏度 (1-10)
  uint16_t  sleepTimeoutMs;       // Deep Sleep 超时 (ms)
  uint16_t  bleTimeoutMs;         // BLE 空闲超时 (ms)
  uint8_t   reserved[16];         // 预留扩展
};

class ConfigManager {
public:
  /**
   * 初始化，绑定存储管理器
   */
  void begin(StorageManager* storage);

  /**
   * 从 NVS 加载配置
   */
  void load();

  /**
   * 保存配置到 NVS
   */
  void save();

  /**
   * 恢复默认配置
   */
  void reset();

  /**
   * 获取配置引用（可修改后调用 save()）
   */
  DeviceConfig& get();

private:
  StorageManager* _storage = nullptr;
  DeviceConfig    _config;

  void _setDefaults();
};

#endif // CONFIG_MANAGER_H
