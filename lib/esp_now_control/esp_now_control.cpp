#include "esp_now_control.h"
#include "global_config.h"
#include <WiFi.h>
#include <string.h>

// ============================================================
//  ESP-NOW 初始化
// ============================================================

bool ESPNowControl::begin(const uint8_t* peerMAC) {
  memcpy(_peerMAC, peerMAC, 6);

  // ESP-NOW 需要 Wi-Fi 处于 STA 模式（不需要连接路由器）
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();   // 确保不连接任何 AP

  if (esp_now_init() != ESP_OK) {
#if DEBUG_SERIAL
    Serial.println("[ESP-NOW] 初始化失败!");
#endif
    return false;
  }

  // 注册发送完成回调
  esp_now_register_send_cb(_onSent);

  // 添加对等设备（接收端）
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, _peerMAC, 6);
  peerInfo.channel = 0;   // 使用当前 Wi-Fi 通道
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
#if DEBUG_SERIAL
    Serial.println("[ESP-NOW] 添加对端失败!");
#endif
    return false;
  }

  _initialized = true;
#if DEBUG_SERIAL
  Serial.println("[ESP-NOW] 初始化完成，对端已添加");
#endif
  return true;
}

// ============================================================
//  发送命令
// ============================================================

bool ESPNowControl::sendCommand(Cmd cmd) {
  if (!_initialized) return false;

  uint8_t data = static_cast<uint8_t>(cmd);
  esp_err_t result = esp_now_send(_peerMAC, &data, 1);

#if DEBUG_SERIAL
  if (result == ESP_OK) {
    Serial.print("[ESP-NOW] 已发送命令: 0x");
    Serial.println(data, HEX);
  } else {
    Serial.println("[ESP-NOW] 发送失败!");
  }
#endif

  return (result == ESP_OK);
}

// ============================================================
//  发送完成回调
// ============================================================

void ESPNowControl::_onSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
#if DEBUG_SERIAL
  if (status != ESP_NOW_SEND_SUCCESS) {
    Serial.println("[ESP-NOW] 对端未确认接收");
  }
#endif
}
