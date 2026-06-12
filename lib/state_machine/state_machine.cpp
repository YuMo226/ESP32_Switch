#include "state_machine.h"
#include "encoder_manager.h"
#include "button_manager.h"
#include "esp_now_manager.h"
#include "ble_manager.h"
#include "battery_manager.h"
#include "power_manager.h"
#include "sleep_manager.h"
#include "config_manager.h"

// ============================================================
//  初始化
// ============================================================

void StateMachine::begin(EventManager* em,
                         EncoderManager* enc, ButtonManager* btn,
                         EspNowManager* espNow, BleManager* ble,
                         BatteryManager* batt, PowerManager* power,
                         SleepManager* sleep, ConfigManager* config) {
  _events  = em;
  _encoder = enc;
  _button  = btn;
  _espNow  = espNow;
  _ble     = ble;
  _battery = batt;
  _power   = power;
  _sleep   = sleep;
  _config  = config;

  _enterState(State::BOOT);
}

// ============================================================
//  主循环驱动
// ============================================================

void StateMachine::update() {
  // 1. 检查超时
  _checkTimeouts();

  // 2. 消费事件队列
  Event e;
  while (_events->pop(e)) {
#if DEBUG_SERIAL
    Serial.printf("[SM] 事件: %s (delta=%d)\n",
                  EventManager::toString(e.type), e.delta);
#endif
    // 每个事件都重置活动计时器（除系统事件外）
    if (e.type >= EventType::ROTATE_CW && e.type <= EventType::BUTTON_DOUBLE_CLICK) {
      _touchActivity();
    }

    // 分发到当前状态的处理函数
    switch (_state) {
      case State::BOOT:         _handleBoot(e);         break;
      case State::IDLE:         _handleIdle(e);         break;
      case State::ACTIVE:       _handleActive(e);       break;
      case State::ESPNOW_TX:    _handleEspNowTx(e);     break;
      case State::BLE_MODE:     _handleBleMode(e);      break;
      case State::CONFIG_MODE:  _handleConfigMode(e);   break;
      case State::LOW_BATTERY:  _handleLowBattery(e);   break;
      case State::DEEP_SLEEP:   _handleDeepSleep(e);    break;
    }
  }

  // 3. 状态特定的持续逻辑
  switch (_state) {
    case State::ACTIVE:
      // ESP-NOW 聚合窗口到期 → 切换到 ESPNOW_TX
      if (_espNow && _espNow->isBatching() && _espNow->isWindowExpired()) {
        _enterState(State::ESPNOW_TX);
      }
      break;
    case State::BLE_MODE:
      // BLE 60s 无操作 → 回到 IDLE
      if (_ble && _ble->isIdle()) {
        _enterState(State::IDLE);
      }
      break;
    default:
      break;
  }
}

// ============================================================
//  状态转换
// ============================================================

void StateMachine::_enterState(State newState) {
  State oldState = _state;
  _exitState(oldState);

  _state = newState;
  _stateEntryTime = millis();

#if DEBUG_SERIAL
  Serial.printf("[State] %s → %s\n", stateName(oldState), stateName(newState));
#endif

  // 状态进入钩子
  switch (newState) {
    case State::BOOT:
      // BOOT 状态立即完成初始化，进入 IDLE
      // 实际初始化在 main.cpp setup() 中完成
      // 这里只是框架入口
      _enterState(State::IDLE);
      return;   // 注意：递归调用，直接返回

    case State::IDLE:
      // 降低 CPU 频率，关闭无线
      if (_power) _power->setCpuFreq(40);
      _touchActivity();   // 重置空闲计时器，防止立即进入 DEEP_SLEEP
      break;

    case State::ACTIVE:
      // 提升 CPU 频率
      if (_power) _power->setCpuFreq(CPU_FREQ_MHZ);
      _touchActivity();   // 重置空闲计时器
      break;

    case State::ESPNOW_TX:
      // TODO: 发送 ESP-NOW 数据包
      // 发送完成后立即返回 ACTIVE
#if DEBUG_SERIAL
      Serial.println("[ESPNOW_TX] 准备发送数据...");
#endif
      // 框架：发送完成后切回 ACTIVE
      _enterState(State::ACTIVE);
      return;

    case State::BLE_MODE:
      // 启动 BLE 栈
      if (_ble) _ble->activate();
      _touchActivity();   // 重置 BLE 空闲计时器
#if DEBUG_SERIAL
      Serial.println("[BLE_MODE] BLE HID 已启动");
#endif
      break;

    case State::CONFIG_MODE:
      // TODO: 进入配置模式
#if DEBUG_SERIAL
      Serial.println("[CONFIG_MODE] 等待配置...");
#endif
      break;

    case State::LOW_BATTERY:
      // 降低功耗
      if (_power) _power->setCpuFreq(40);
#if DEBUG_SERIAL
      Serial.println("[LOW_BATTERY] 进入低电量保护模式");
#endif
      break;

    case State::DEEP_SLEEP:
      // 准备并进入深度睡眠
      if (_encoder && _sleep) {
        _sleep->prepareSleep(_encoder->getState());
      }
#if DEBUG_SERIAL
      Serial.println("[DEEP_SLEEP] 准备进入深度睡眠...");
#endif
      if (_sleep) _sleep->enterDeepSleep();
      break;
  }
}

void StateMachine::_exitState(State oldState) {
  switch (oldState) {
    case State::BLE_MODE:
      // 关闭 BLE 栈
      if (_ble) _ble->deactivate();
#if DEBUG_SERIAL
      Serial.println("[BLE_MODE] BLE 已关闭");
#endif
      break;
    case State::CONFIG_MODE:
      // 保存配置
      if (_config) _config->save();
      break;
    case State::LOW_BATTERY:
      // 恢复正常 CPU 频率
      if (_power) _power->setCpuFreq(CPU_FREQ_MHZ);
      break;
    default:
      break;
  }
}

// ============================================================
//  各状态的事件处理
// ============================================================

void StateMachine::_handleBoot(const Event& e) {
  // BOOT 状态不处理事件（初始化已在 _enterState 中完成）
  // 理论上不会收到事件
}

void StateMachine::_handleIdle(const Event& e) {
  switch (e.type) {
    case EventType::ROTATE_CW:
    case EventType::ROTATE_CCW:
    case EventType::BUTTON_PRESS:
    case EventType::BUTTON_LONG_PRESS:
    case EventType::BUTTON_DOUBLE_CLICK:
      // 任何用户输入 → 进入 ACTIVE
      _enterState(State::ACTIVE);
      // 重新推入事件让 ACTIVE 处理（不丢弃）
      _events->push(e.type, e.delta);
      break;

    case EventType::BATTERY_LOW:
      _enterState(State::LOW_BATTERY);
      break;

    default:
      break;
  }
}

void StateMachine::_handleActive(const Event& e) {
  switch (e.type) {
    case EventType::ROTATE_CW:
    case EventType::ROTATE_CCW:
      // 旋转事件 → 交给 ESP-NOW 聚合
      if (_espNow) {
        _espNow->startBatching();
        _espNow->addRotation(e.delta);
      }
      break;

    case EventType::BUTTON_PRESS:
      // 短按 → ESP-NOW 发送按键事件
      if (_espNow) {
        uint8_t bat = _battery ? _battery->readPercent() : 0;
        _espNow->sendButtonEvent(PktEventType::PRESS, bat);
      }
      break;

    case EventType::BUTTON_LONG_PRESS:
      // 长按 → 切换到 BLE 模式
      _enterState(State::BLE_MODE);
      break;

    case EventType::BUTTON_DOUBLE_CLICK:
      // 双击 → ESP-NOW 发送
      if (_espNow) {
        uint8_t bat = _battery ? _battery->readPercent() : 0;
        _espNow->sendButtonEvent(PktEventType::DBL_CLICK, bat);
      }
      break;

    case EventType::BATTERY_LOW:
      _enterState(State::LOW_BATTERY);
      break;

    default:
      break;
  }
}

void StateMachine::_handleEspNowTx(const Event& e) {
  // ESPNOW_TX 是瞬态，_enterState 已自动切回 ACTIVE
  // 不会收到事件
}

void StateMachine::_handleBleMode(const Event& e) {
  switch (e.type) {
    case EventType::ROTATE_CW:
      if (_ble) _ble->sendVolumeUp();
      break;
    case EventType::ROTATE_CCW:
      if (_ble) _ble->sendVolumeDown();
      break;
    case EventType::BUTTON_PRESS:
      if (_ble) _ble->sendMute();
      break;
    case EventType::BUTTON_LONG_PRESS:
      // 再次长按 → 退出 BLE，回到 IDLE
      _enterState(State::IDLE);
      break;
    case EventType::BATTERY_LOW:
      // BLE 模式下不强制退出，让用户完成当前操作
      // 只打印警告，不切换状态
#if DEBUG_SERIAL
      Serial.println("[BLE_MODE] 警告: 电量低，但继续 BLE 操作");
#endif
      break;
    default:
      break;
  }
}

void StateMachine::_handleConfigMode(const Event& e) {
  // TODO: 配置模式的具体交互逻辑
  // 例如：旋转修改参数，短按确认，长按退出
  switch (e.type) {
    case EventType::BUTTON_LONG_PRESS:
      // 长按 → 退出配置模式
      _enterState(State::IDLE);
      break;
    default:
      break;
  }
}

void StateMachine::_handleLowBattery(const Event& e) {
  // 低电量模式下，限制功能但允许基本操作
  switch (e.type) {
    case EventType::BATTERY_NORMAL:
      // 电量恢复 → 回到 IDLE
      _enterState(State::IDLE);
      break;
    case EventType::ROTATE_CW:
    case EventType::ROTATE_CCW:
    case EventType::BUTTON_PRESS:
      // 允许基本操作，但限制频率
      // TODO: 降低 ESP-NOW 发送频率
      break;
    case EventType::BUTTON_LONG_PRESS:
      // 长按 → 切换到 BLE 模式（低电量下也允许）
      _enterState(State::BLE_MODE);
      break;
    case EventType::BUTTON_DOUBLE_CLICK:
      // 双击 → 发送 ESP-NOW（低电量下也允许）
      if (_espNow) {
        uint8_t bat = _battery ? _battery->readPercent() : 0;
        _espNow->sendButtonEvent(PktEventType::DBL_CLICK, bat);
      }
      break;
    default:
      break;
  }
}

void StateMachine::_handleDeepSleep(const Event& e) {
  // DEEP_SLEEP 状态不会收到事件（CPU 即将关闭）
}

// ============================================================
//  超时检查
// ============================================================

void StateMachine::_checkTimeouts() {
  unsigned long now = millis();
  unsigned long idle = now - _lastActivity;

  // 电池检查（BLE_MODE 下不检查，避免频繁切换）
  if (_battery && _state != State::DEEP_SLEEP && _state != State::LOW_BATTERY && _state != State::BLE_MODE) {
    if (_battery->isLow()) {
      _events->push(EventType::BATTERY_LOW);
    }
  }

  // 状态特定超时
  switch (_state) {
    case State::ACTIVE:
      // 5s 无操作 → IDLE
      if (idle > IDLE_TIMEOUT_MS) {
        _enterState(State::IDLE);
      }
      break;

    case State::IDLE:
      // IDLE 状态持续 5s 无事件 → DEEP_SLEEP
      if (idle > IDLE_TIMEOUT_MS) {
        _enterState(State::DEEP_SLEEP);
      }
      break;

    default:
      break;
  }
}

// ============================================================
//  辅助
// ============================================================

void StateMachine::_touchActivity() {
  _lastActivity = millis();
}

State StateMachine::getCurrentState() const {
  return _state;
}

const char* StateMachine::stateName(State s) {
  switch (s) {
    case State::BOOT:         return "BOOT";
    case State::IDLE:         return "IDLE";
    case State::ACTIVE:       return "ACTIVE";
    case State::ESPNOW_TX:    return "ESPNOW_TX";
    case State::BLE_MODE:     return "BLE_MODE";
    case State::CONFIG_MODE:  return "CONFIG_MODE";
    case State::LOW_BATTERY:  return "LOW_BATTERY";
    case State::DEEP_SLEEP:   return "DEEP_SLEEP";
    default:                  return "UNKNOWN";
  }
}
