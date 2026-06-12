#ifndef ESP_NOW_MANAGER_H
#define ESP_NOW_MANAGER_H

#include <Arduino.h>
#include <esp_now.h>
#include "config.h"

// ============================================================
//  ESP-NOW 管理器 — 事件聚合 + 低功耗发送
//
//  核心策略：100ms 聚合窗口内合并所有旋转增量，
//  窗口到期后发一次包，然后关闭 WiFi 射频。
// ============================================================

// 事件类型编码（数据包用）
enum class PktEventType : uint8_t {
  ROTATE      = 0x01,
  PRESS       = 0x02,
  LONG_PRESS  = 0x03,
  DBL_CLICK   = 0x04,
};

class EspNowManager {
public:
  /**
   * 初始化 ESP-NOW，添加对端
   */
  bool begin(const uint8_t* peerMAC);

  /**
   * 开始新的聚合窗口（收到第一个事件时调用）
   */
  void startBatching();

  /**
   * 累加旋转增量
   */
  void addRotation(int delta);

  /**
   * 发送按键事件（立即发送，不聚合）
   */
  void sendButtonEvent(PktEventType type, uint8_t batteryPct);

  /**
   * 聚合窗口是否到期 (100ms)
   */
  bool isWindowExpired() const;

  /**
   * 发送聚合数据包 + 关闭 WiFi
   */
  void flush(uint8_t batteryPct);

  /**
   * 是否正在聚合（窗口活跃）
   */
  bool isBatching() const;

  /**
   * WiFi 是否已初始化
   */
  bool isActive() const;

private:
  uint8_t       _peerMAC[6]   = {};
  bool          _initialized  = false;
  bool          _batching     = false;
  int           _batchDelta   = 0;
  unsigned long _batchStart   = 0;

  void _sendPacket(PktEventType type, int delta, uint8_t batteryPct);
  static void _onSent(const uint8_t* mac_addr, esp_now_send_status_t status);

  /**
   * 使用 ESP-IDF 底层 API 初始化 WiFi
   * 比 Arduino WiFi 库更可控，避免卡死
   */
  bool _initWiFiLowLevel();
};

#endif // ESP_NOW_MANAGER_H
