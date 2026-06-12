#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>
#include "config.h"

class EventManager;   // 前向声明

// ============================================================
//  按键管理器 — 短按 / 长按 / 双击
//
//  状态机实现，不阻塞主循环。
//  按键 GPIO 中断仅设置标志，复杂逻辑在 update() 中处理。
// ============================================================

class ButtonManager {
public:
  /**
   * 初始化 GPIO + 中断
   */
  void begin(EventManager* em);

  /**
   * 在 loop() 中调用，处理按键状态机
   */
  void update();

private:
  EventManager* _em = nullptr;

  // 中断标志
  volatile bool _pressFlag     = false;
  volatile bool _releaseFlag   = false;

  // 状态机
  enum class BtnState : uint8_t { IDLE, PRESSED, WAIT_DOUBLE };
  BtnState      _state         = BtnState::IDLE;
  unsigned long _pressStart    = 0;
  unsigned long _releaseTime   = 0;
  bool          _longPressSent = false;

  // 中断服务函数
  static void IRAM_ATTR _btnISR(void* arg);
};

#endif // BUTTON_MANAGER_H
