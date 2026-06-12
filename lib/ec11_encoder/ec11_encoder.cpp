#include "ec11_encoder.h"
#include "global_config.h"

// ============================================================
//  状态机查表法 — 四倍频解码
//
//  EC11 编码器 A/B 两相产生正交信号（格雷码），
//  合法的状态转换序列：
//    顺时针: 00 → 01 → 11 → 10 → 00  (0→1→3→2→0)
//    逆时针: 00 → 10 → 11 → 01 → 00  (0→2→3→1→0)
//  跳过中间状态（如 00→11）= 抖动，丢弃。
//
//  表索引: prevState*4 + currState
//    0 = 无效,  1 = CW,  2 = CCW
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

void EC11Encoder::begin() {
  // EC11 内部是机械触点开关，需要上拉保持空闲 HIGH
  // 如果模块板载外部上拉，可改为 INPUT
  pinMode(PIN_ENC_A,  INPUT_PULLUP);
  pinMode(PIN_ENC_B,  INPUT_PULLUP);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  // 读取初始 A/B 相位
  _prevEncState = _readAB();

  // 挂载中断 — 用 attachInterruptArg 传入 this 指针
  attachInterruptArg(digitalPinToInterrupt(PIN_ENC_A), _encoderISR, this, CHANGE);
  attachInterruptArg(digitalPinToInterrupt(PIN_ENC_B), _encoderISR, this, CHANGE);
  attachInterruptArg(digitalPinToInterrupt(PIN_BUTTON), _buttonISR, this, FALLING);

#if DEBUG_SERIAL
  Serial.println("[EC11] 初始化完成");
#endif
}

// ============================================================
//  主循环调用 — 非阻塞事件检查
// ============================================================

EC11Event EC11Encoder::update() {
  // 优先处理旋转事件
  if (_encMoved) {
    noInterrupts();
    int dir = _lastDir;
    _encMoved = false;
    interrupts();

    return (dir > 0) ? EC11Event::CW : EC11Event::CCW;
  }

  // 再处理按键事件
  if (_btnPressed) {
    _btnPressed = false;
    return EC11Event::PRESSED;
  }

  return EC11Event::NONE;
}

int EC11Encoder::getPosition() const {
  return _pos;
}

void EC11Encoder::resetPosition() {
  noInterrupts();
  _pos = 0;
  _tickCount = 0;
  interrupts();
}

void EC11Encoder::setPosition(int pos) {
  noInterrupts();
  _pos = pos;
  if (_pos < POS_MIN) _pos = POS_MIN;
  if (_pos > POS_MAX) _pos = POS_MAX;
  _tickCount = 0;
  interrupts();
}

// ============================================================
//  中断服务函数
// ============================================================

// 读取 A/B 相位（内联辅助）
uint8_t EC11Encoder::_readAB() {
  return (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);
}

// 编码器旋转中断 — A/B 相 CHANGE 触发
void IRAM_ATTR EC11Encoder::_encoderISR(void* arg) {
  EC11Encoder* self = static_cast<EC11Encoder*>(arg);

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
    return;  // 非法跳变，不处理
  }

  // 每 4 次四倍频跳变 = 1 个物理咔嗒 → 更新位置
  if (self->_tickCount >= 4 || self->_tickCount <= -4) {
    int steps = self->_tickCount / 4;
    self->_tickCount -= steps * 4;   // 保留余数，避免累积误差

    self->_pos += steps;
    // 钳位到 0~100
    if (self->_pos < POS_MIN) self->_pos = POS_MIN;
    if (self->_pos > POS_MAX) self->_pos = POS_MAX;
    self->_encMoved = true;
  }
}

// 按键中断 — 下降沿触发
void IRAM_ATTR EC11Encoder::_buttonISR(void* arg) {
  EC11Encoder* self = static_cast<EC11Encoder*>(arg);

  unsigned long now = millis();
  if (now - self->_lastBtnPress < BTN_DEBOUNCE_MS) return;
  self->_lastBtnPress = now;

  self->_btnPressed = true;
}
