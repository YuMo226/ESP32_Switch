#include "button_manager.h"
#include "event_manager.h"

void ButtonManager::begin(EventManager* em) {
  _em = em;

  pinMode(PIN_BUTTON, INPUT_PULLUP);

  // 下降沿 = 按下，上升沿 = 释放
  attachInterruptArg(digitalPinToInterrupt(PIN_BUTTON), _btnISR, this, CHANGE);

#if DEBUG_SERIAL
  Serial.println("[Button] 初始化完成");
#endif
}

void ButtonManager::update() {
  unsigned long now = millis();

  // 处理中断标志
  if (_pressFlag) {
    _pressFlag = false;
    if (_state == BtnState::IDLE) {
      _state = BtnState::PRESSED;
      _pressStart = now;
      _longPressSent = false;
    } else if (_state == BtnState::WAIT_DOUBLE) {
      // 第二次按下 → 双击
      _state = BtnState::IDLE;
      if (_em) _em->push(EventType::BUTTON_DOUBLE_CLICK);
#if DEBUG_SERIAL
      Serial.println("[Button] 双击");
#endif
      return;
    }
  }

  if (_releaseFlag) {
    _releaseFlag = false;
    if (_state == BtnState::PRESSED) {
      unsigned long duration = now - _pressStart;

      if (duration >= LONG_PRESS_MS) {
        // 已经在按下期间发送过 LONG_PRESS，不再发 PRESS
        _state = BtnState::IDLE;
      } else {
        // 短按释放，进入等待双击窗口
        _state = BtnState::WAIT_DOUBLE;
        _releaseTime = now;
      }
    }
  }

  // 长按检测（在按下期间持续检测）
  if (_state == BtnState::PRESSED && !_longPressSent) {
    if (now - _pressStart >= LONG_PRESS_MS) {
      _longPressSent = true;
      if (_em) _em->push(EventType::BUTTON_LONG_PRESS);
#if DEBUG_SERIAL
      Serial.println("[Button] 长按");
#endif
    }
  }

  // 双击等待窗口超时 → 单击
  if (_state == BtnState::WAIT_DOUBLE) {
    if (now - _releaseTime >= DOUBLE_CLICK_MS) {
      _state = BtnState::IDLE;
      if (_em) _em->push(EventType::BUTTON_PRESS);
#if DEBUG_SERIAL
      Serial.println("[Button] 短按");
#endif
    }
  }
}

// ============================================================
//  按键中断 — 仅设置标志，不做复杂逻辑
// ============================================================

void IRAM_ATTR ButtonManager::_btnISR(void* arg) {
  ButtonManager* self = static_cast<ButtonManager*>(arg);

  static unsigned long lastTime = 0;
  unsigned long now = millis();
  if (now - lastTime < BTN_DEBOUNCE_MS) return;
  lastTime = now;

  if (digitalRead(PIN_BUTTON) == LOW) {
    self->_pressFlag = true;     // 按下
  } else {
    self->_releaseFlag = true;   // 释放
  }
}
