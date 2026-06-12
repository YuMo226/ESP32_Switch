#ifndef RADIO_MANAGER_H
#define RADIO_MANAGER_H

#include <Arduino.h>
#include "ble_hid_control.h"
#include "esp_now_control.h"

// ============================================================
//  无线路由调度核心 (Radio Manager)
//
//  职责：决定旋钮信号走哪条无线通路
//
//    ┌── BLE 已连接 ──→ 走蓝牙 HID 直连电脑
//    │
//    ├── ESP-NOW 对端在线 ──→ 走 ESP-NOW 发给接收端
//    │
//    └── 都不可用 ──→ 仅本地串口输出
//
//  主程序只调用 handleLocalXxx()，不关心底层走哪条路。
// ============================================================

// 路由结果（调试用）
enum class RouteTarget : uint8_t {
  NONE      = 0,    // 无可用通路
  BLE       = 1,    // 走蓝牙
  ESP_NOW   = 2,    // 走 ESP-NOW
};

class RadioManager {
public:
  /**
   * 初始化所有无线子模块
   * @param peerMAC  ESP-NOW 接收端 MAC 地址
   */
  void begin(const uint8_t* peerMAC);

  /**
   * 主循环维护（BLE 协议栈需要持续调用）
   */
  void loop();

  // ---- 统一接口：主程序只调用这些 ----

  /**
   * 处理本地音量调节（旋转旋钮触发）
   * @param delta  +1 = 音量加，-1 = 音量减
   */
  void handleLocalVolume(int delta);

  /**
   * 处理本地开关机动作（按键触发）
   */
  void handleLocalPowerToggle();

  /**
   * 处理本地静音（长按或双击触发）
   */
  void handleLocalMute();

  // ---- 状态查询 ----

  /**
   * 当前使用的是哪条通路
   */
  RouteTarget getActiveRoute() const;

  /**
   * BLE 是否已连接主机
   */
  bool isBLEConnected() const;

private:
  BLEHIDControl  _ble;
  ESPNowControl  _espNow;
  RouteTarget    _activeRoute = RouteTarget::NONE;

  /**
   * 内部路由决策：选择当前最优通路
   * 优先级：BLE > ESP-NOW > NONE
   */
  void _updateRoute();
};

#endif // RADIO_MANAGER_H
