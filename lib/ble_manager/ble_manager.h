#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include "config.h"

// ============================================================
//  BLE 管理器 — 按需启动的备用通信模式
//
//  默认不启动 BLE 栈（节省 RAM + 电流）。
//  仅在用户长按按键时激活。
//  60 秒无操作自动关闭。
// ============================================================

class BleManager {
public:
  /**
   * 启动 BLE HID 栈（~2-3 秒初始化）
   * 仅在需要时调用，不要在 setup() 中调用
   */
  void activate();

  /**
   * 关闭 BLE 栈，释放资源
   */
  void deactivate();

  /**
   * BLE 栈是否已启动
   */
  bool isActive() const;

  /**
   * 是否已连接主机
   */
  bool isConnected() const;

  /**
   * 60 秒无操作
   */
  bool isIdle() const;

  /**
   * 更新活跃时间戳（有事件时调用）
   */
  void touch();

  /**
   * 多媒体控制
   */
  void sendVolumeUp();
  void sendVolumeDown();
  void sendMute();

private:
  bool          _initialized = false;
  unsigned long _lastActive  = 0;
};

#endif // BLE_MANAGER_H
