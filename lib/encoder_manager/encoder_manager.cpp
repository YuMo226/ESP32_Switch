#include "encoder_manager.h"
#include "event_manager.h"

// ============================================================
//  状态机查表法 — 四倍频解码
//  表索引: prevState*4 + currState
//  0 = 无效,  1 = CW,  2 = CCW
// ============================================================

static const int8_t encTable[16] = {
  //  curr:  00  01  10  11
  /* prev 00 */  0,  1,  2,  0,
  /* prev 01 */  2,  0,  0,  1,
  /* prev 10 */  1,  0,  0,  2,
  /* prev 11 */  0,  2,  1,  0,
};

// ============================================================
//  初始化
// ============================================================

void EncoderManager::begin(EventManager* em) {
  _em = em;

  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);

  _prevEncState = _readAB();

  // A、B 相都挂 CHANGE 中断（四倍频）
  attachInterruptArg(digitalPinToInterrupt(PIN_ENC_A), _encoderISR, this, CHANGE);
  attachInterruptArg(digitalPinToInterrupt(PIN_ENC_B), _encoderISR, this, CHANGE);

#if DEBUG_SERIAL
  Serial.println("[Encoder] 初始化完成");
#endif
}

// ============================================================
//  主循环调用 — 检查中断标志，推入事件队列
// ============================================================

void EncoderManager::update() {
  if (!_encMoved) return;

  noInterrupts();
  int dir = _lastDir;
  _encMoved = false;
  interrupts();

  if (_em) {
    _em->push(dir > 0 ? EventType::ROTATE_CW : EventType::ROTATE_CCW, dir);
  }
}

int EncoderManager::getPosition() const {
  return _pos;
}

// ============================================================
//  Deep Sleep 支持 — 保存/恢复 A/B 相位
// ============================================================

uint8_t EncoderManager::getState() const {
  return _prevEncState;
}

void EncoderManager::restoreState(uint8_t savedState) {
  _prevEncState = savedState;
}

// ============================================================
//  内部方法
// ============================================================

uint8_t EncoderManager::_readAB() {
  return (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);
}

// ============================================================
//  中断服务函数
// ============================================================

void IRAM_ATTR EncoderManager::_encoderISR(void* arg) {
  EncoderManager* self = static_cast<EncoderManager*>(arg);

  static unsigned long lastTime = 0;
  unsigned long now = micros();
  if (now - lastTime < ENC_DEBOUNCE_US) return;
  lastTime = now;

  uint8_t currState = self->_readAB();
  uint8_t idx = (self->_prevEncState << 2) | currState;
  self->_prevEncState = currState;

  int8_t result = encTable[idx];
  if (result == 1) {
    self->_tickCount--;
    self->_lastDir = -1;
  } else if (result == 2) {
    self->_tickCount++;
    self->_lastDir = 1;
  } else {
    return;
  }

  // 每 4 次跳变 = 1 个物理咔嗒
  if (self->_tickCount >= PULSES_PER_DETENT || self->_tickCount <= -PULSES_PER_DETENT) {
    int steps = self->_tickCount / PULSES_PER_DETENT;
    self->_tickCount -= steps * PULSES_PER_DETENT;

    self->_pos += steps;
    if (self->_pos < ENC_POS_MIN) self->_pos = ENC_POS_MIN;
    if (self->_pos > ENC_POS_MAX) self->_pos = ENC_POS_MAX;
    self->_encMoved = true;
  }
}
