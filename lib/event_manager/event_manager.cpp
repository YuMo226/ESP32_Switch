#include "event_manager.h"

void EventManager::push(EventType type, int delta) {
  uint8_t next = (_head + 1) % EVENT_QUEUE_SIZE;
  if (next == _tail) return;   // 队列满，丢弃
  _queue[_head] = {type, delta};
  _head = next;
}

bool EventManager::pop(Event& out) {
  if (_head == _tail) return false;
  out = _queue[_tail];
  _tail = (_tail + 1) % EVENT_QUEUE_SIZE;
  return true;
}

bool EventManager::hasEvents() const {
  return _head != _tail;
}

void EventManager::clear() {
  _head = 0;
  _tail = 0;
}

const char* EventManager::toString(EventType type) {
  switch (type) {
    case EventType::NONE:               return "NONE";
    case EventType::SYSTEM_BOOT:        return "SYSTEM_BOOT";
    case EventType::WAKEUP:             return "WAKEUP";
    case EventType::ENTER_SLEEP:        return "ENTER_SLEEP";
    case EventType::ROTATE_CW:          return "ROTATE_CW";
    case EventType::ROTATE_CCW:         return "ROTATE_CCW";
    case EventType::BUTTON_PRESS:       return "BUTTON_PRESS";
    case EventType::BUTTON_RELEASE:     return "BUTTON_RELEASE";
    case EventType::BUTTON_LONG_PRESS:  return "BUTTON_LONG_PRESS";
    case EventType::BUTTON_DOUBLE_CLICK:return "BUTTON_DOUBLE_CLICK";
    case EventType::BATTERY_LOW:        return "BATTERY_LOW";
    case EventType::BATTERY_NORMAL:     return "BATTERY_NORMAL";
    case EventType::ENTER_BLE_MODE:     return "ENTER_BLE_MODE";
    case EventType::EXIT_BLE_MODE:      return "EXIT_BLE_MODE";
    case EventType::BLE_CONNECTED:      return "BLE_CONNECTED";
    case EventType::BLE_DISCONNECTED:   return "BLE_DISCONNECTED";
    case EventType::ESP_NOW_SEND:       return "ESP_NOW_SEND";
    case EventType::ESP_NOW_SUCCESS:    return "ESP_NOW_SUCCESS";
    case EventType::ESP_NOW_FAIL:       return "ESP_NOW_FAIL";
    case EventType::ENTER_CONFIG_MODE:  return "ENTER_CONFIG_MODE";
    case EventType::EXIT_CONFIG_MODE:   return "EXIT_CONFIG_MODE";
    default:                            return "UNKNOWN";
  }
}
