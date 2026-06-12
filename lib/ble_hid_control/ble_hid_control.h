#ifndef BLE_HID_CONTROL_H
#define BLE_HID_CONTROL_H

#include <Arduino.h>

// ============================================================
//  蓝牙 BLE HID 控制模块
//
//  将 ESP32-C3 虚拟为蓝牙 Consumer Control 设备，
//  实现多媒体键控制（音量加/减/静音）。
//
//  使用方式:
//    BLEHIDControl ble;
//    ble.begin();
//    // 检查连接状态
//    if (ble.isConnected()) { ... }
//    // 发送音量控制
//    ble.sendVolumeUp();
// ============================================================

class BLEHIDControl {
public:
  /**
   * 初始化 BLE HID 设备
   * 设备名称在 global_config.h 的 BLE_DEVICE_NAME 中定义
   */
  void begin();

  /**
   * BLE 需要在 loop 中持续维护连接
   */
  void loop();

  /**
   * 检查是否有主机（电脑/手机）已连接
   */
  bool isConnected() const;

  /**
   * 发送音量增加（Consumer Control: Volume Up）
   */
  void sendVolumeUp();

  /**
   * 发送音量减少（Consumer Control: Volume Down）
   */
  void sendVolumeDown();

  /**
   * 发送静音切换（Consumer Control: Mute）
   */
  void sendMute();

private:
  bool _initialized = false;
};

#endif // BLE_HID_CONTROL_H
