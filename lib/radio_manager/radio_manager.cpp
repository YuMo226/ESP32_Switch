#include "radio_manager.h"
#include "global_config.h"

// ============================================================
//  初始化
// ============================================================

void RadioManager::begin(const uint8_t* peerMAC) {
#if DEBUG_SERIAL
  Serial.println("[Radio] 正在初始化无线管理器...");
#endif

  // 1. 启动 BLE HID
  _ble.begin();

  // 2. 启动 ESP-NOW
  _espNow.begin(peerMAC);

  // 3. 初始路由决策
  _updateRoute();

#if DEBUG_SERIAL
  Serial.print("[Radio] 当前路由: ");
  switch (_activeRoute) {
    case RouteTarget::BLE:     Serial.println("BLE 蓝牙"); break;
    case RouteTarget::ESP_NOW: Serial.println("ESP-NOW"); break;
    case RouteTarget::NONE:    Serial.println("无（仅串口）"); break;
  }
#endif
}

// ============================================================
//  主循环维护
// ============================================================

void RadioManager::loop() {
  // BLE 协议栈需要持续处理内部事件
  _ble.loop();

  // 定期检查路由状态（BLE 可能随时断开/重连）
  _updateRoute();
}

// ============================================================
//  统一接口 — 音量控制
// ============================================================

void RadioManager::handleLocalVolume(int delta) {
  _updateRoute();

  switch (_activeRoute) {
    case RouteTarget::BLE:
      // 蓝牙直连电脑 → 发送 HID 多媒体键
      if (delta > 0) _ble.sendVolumeUp();
      else           _ble.sendVolumeDown();
      break;

    case RouteTarget::ESP_NOW:
      // ESP-NOW → 发给接收端中转
      if (delta > 0) _espNow.sendCommand(Cmd::VOL_UP);
      else           _espNow.sendCommand(Cmd::VOL_DOWN);
      break;

    case RouteTarget::NONE:
    default:
      // 无通路，仅串口输出（main.cpp 已处理）
      break;
  }
}

// ============================================================
//  统一接口 — 开关机
// ============================================================

void RadioManager::handleLocalPowerToggle() {
  _updateRoute();

  switch (_activeRoute) {
    case RouteTarget::BLE:
      // BLE 模式下，按键也可以发送特定键（如 Media Play/Pause）
      // 这里暂不映射，开关机走 ESP-NOW 更合理
      // 如果 BLE 是唯一通路，可以映射为某个自定义键
#if DEBUG_SERIAL
      Serial.println("[Radio] BLE 模式下开关机命令未映射");
#endif
      break;

    case RouteTarget::ESP_NOW:
      _espNow.sendCommand(Cmd::POWER_ON);
      break;

    case RouteTarget::NONE:
    default:
      break;
  }
}

// ============================================================
//  统一接口 — 静音
// ============================================================

void RadioManager::handleLocalMute() {
  _updateRoute();

  switch (_activeRoute) {
    case RouteTarget::BLE:
      _ble.sendMute();
      break;

    case RouteTarget::ESP_NOW:
      _espNow.sendCommand(Cmd::MUTE);
      break;

    case RouteTarget::NONE:
    default:
      break;
  }
}

// ============================================================
//  状态查询
// ============================================================

RouteTarget RadioManager::getActiveRoute() const {
  return _activeRoute;
}

bool RadioManager::isBLEConnected() const {
  return _ble.isConnected();
}

// ============================================================
//  路由决策（内部）
//
//  优先级：BLE > ESP-NOW > NONE
//  BLE 连接时优先走蓝牙（低延迟、直连电脑），
//  BLE 断开时自动回退到 ESP-NOW（接收端中转）。
// ============================================================

void RadioManager::_updateRoute() {
#if BLE_ENABLED
  if (_ble.isConnected()) {
    _activeRoute = RouteTarget::BLE;
    return;
  }
#endif

  // ESP-NOW 始终可用（局域广播，不需要握手）
  _activeRoute = RouteTarget::ESP_NOW;
}
