#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "config.h"
#include "event_manager.h"

// 前向声明所有模块
class EncoderManager;
class ButtonManager;
class EspNowManager;
class BleManager;
class BatteryManager;
class PowerManager;
class SleepManager;
class ConfigManager;

// ============================================================
//  系统状态定义
// ============================================================

enum class State : uint8_t {
  BOOT,           // 上电初始化
  IDLE,           // 低功耗待机（默认）
  ACTIVE,         // 用户正在操作
  ESPNOW_TX,      // ESP-NOW 发送中
  BLE_MODE,       // BLE 备用模式
  CONFIG_MODE,    // 配置模式
  LOW_BATTERY,    // 低电量保护
  DEEP_SLEEP,     // 深度睡眠
};

// ============================================================
//  状态机
//
//  职责：
//  - 管理系统状态转换
//  - 消费事件队列
//  - 分发事件到对应模块
//  - 管理状态进入/退出钩子
//
//  不做：
//  - 不实现具体业务逻辑（留给各模块）
//  - 不直接操作硬件（通过模块间接操作）
// ============================================================

class StateMachine {
public:
  /**
   * 初始化状态机，绑定所有模块
   */
  void begin(EventManager* em,
             EncoderManager* enc, ButtonManager* btn,
             EspNowManager* espNow, BleManager* ble,
             BatteryManager* batt, PowerManager* power,
             SleepManager* sleep, ConfigManager* config);

  /**
   * 主循环调用 — 驱动状态机
   * 1. 检查超时
   * 2. 消费事件队列
   * 3. 执行状态特定逻辑
   */
  void update();

  /**
   * 获取当前状态
   */
  State getCurrentState() const;

  /**
   * 获取状态名称（调试用）
   */
  static const char* stateName(State s);

private:
  // ---- 模块指针 ----
  EventManager*   _events   = nullptr;
  EncoderManager* _encoder  = nullptr;
  ButtonManager*  _button   = nullptr;
  EspNowManager*  _espNow   = nullptr;
  BleManager*     _ble      = nullptr;
  BatteryManager* _battery  = nullptr;
  PowerManager*   _power    = nullptr;
  SleepManager*   _sleep    = nullptr;
  ConfigManager*  _config   = nullptr;

  // ---- 状态 ----
  State          _state         = State::BOOT;
  unsigned long  _stateEntryTime = 0;
  unsigned long  _lastActivity   = 0;

  // ---- 状态转换 ----
  void _enterState(State newState);
  void _exitState(State oldState);

  // ---- 事件处理（每状态） ----
  void _handleBoot(const Event& e);
  void _handleIdle(const Event& e);
  void _handleActive(const Event& e);
  void _handleEspNowTx(const Event& e);
  void _handleBleMode(const Event& e);
  void _handleConfigMode(const Event& e);
  void _handleLowBattery(const Event& e);
  void _handleDeepSleep(const Event& e);

  // ---- 超时检查 ----
  void _checkTimeouts();

  // ---- 辅助 ----
  void _touchActivity();
};

#endif // STATE_MACHINE_H
