#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <Arduino.h>
#include "config.h"

// ============================================================
//  事件管理器 — 统一事件队列
//
//  所有输入不直接调用业务逻辑，统一转换为事件推入队列。
//  由状态机统一消费和分发。
// ============================================================

enum class EventType : uint8_t {
  // ---- 系统 ----
  NONE              = 0,
  SYSTEM_BOOT       = 1,
  WAKEUP            = 2,
  ENTER_SLEEP       = 3,

  // ---- 旋转 ----
  ROTATE_CW         = 10,
  ROTATE_CCW        = 11,

  // ---- 按键 ----
  BUTTON_PRESS      = 20,
  BUTTON_RELEASE    = 21,
  BUTTON_LONG_PRESS = 22,
  BUTTON_DOUBLE_CLICK = 23,

  // ---- 电池 ----
  BATTERY_LOW       = 30,
  BATTERY_NORMAL    = 31,

  // ---- BLE ----
  ENTER_BLE_MODE    = 40,
  EXIT_BLE_MODE     = 41,
  BLE_CONNECTED     = 42,
  BLE_DISCONNECTED  = 43,

  // ---- ESP-NOW ----
  ESP_NOW_SEND      = 50,
  ESP_NOW_SUCCESS   = 51,
  ESP_NOW_FAIL      = 52,

  // ---- 配置 ----
  ENTER_CONFIG_MODE = 60,
  EXIT_CONFIG_MODE  = 61,
};

struct Event {
  EventType type;
  int       delta;          // 旋转增量（ROTATE_CW/CCW 时有效）
};

class EventManager {
public:
  void push(EventType type, int delta = 0);
  bool pop(Event& out);
  bool hasEvents() const;
  void clear();

  /**
   * 获取事件类型的调试字符串
   */
  static const char* toString(EventType type);

private:
  Event     _queue[EVENT_QUEUE_SIZE];
  uint8_t   _head = 0;
  uint8_t   _tail = 0;
};

#endif // EVENT_MANAGER_H
