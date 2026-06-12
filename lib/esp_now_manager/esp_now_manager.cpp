#include "esp_now_manager.h"
#include <WiFi.h>

// ESP-IDF 底层 API
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

bool EspNowManager::begin(const uint8_t* peerMAC) {
  memcpy(_peerMAC, peerMAC, 6);

  // 检查 MAC 是否为全零（未配置）
  bool macValid = false;
  for (int i = 0; i < 6; i++) {
    if (_peerMAC[i] != 0) { macValid = true; break; }
  }

#if DEBUG_SERIAL
  Serial.println("[ESP-NOW] 准备初始化 WiFi...");
  Serial.printf("[ESP-NOW] 可用堆内存: %d bytes\n", ESP.getFreeHeap());
  Serial.flush();
#endif

  // 检查可用内存，WiFi 栈需要至少 50KB
  if (ESP.getFreeHeap() < 60000) {
#if DEBUG_SERIAL
    Serial.println("[ESP-NOW] 错误: 堆内存不足，无法初始化 WiFi");
    Serial.flush();
#endif
    _initialized = false;
    return false;
  }

  // 延迟初始化，让电源和系统稳定
  delay(300);

  // 使用 ESP-IDF 底层 API 初始化 WiFi，更可控
  bool wifiInitialized = _initWiFiLowLevel();

  if (!wifiInitialized) {
#if DEBUG_SERIAL
    Serial.println("[ESP-NOW] WiFi 初始化失败，跳过 ESP-NOW");
    Serial.flush();
#endif
    _initialized = false;
    return false;
  }

  // 初始化 ESP-NOW
  if (esp_now_init() != ESP_OK) {
#if DEBUG_SERIAL
    Serial.println("[ESP-NOW] ESP-NOW 初始化失败!");
    Serial.flush();
#endif
    _initialized = false;
    return false;
  }

  esp_now_register_send_cb(_onSent);

  // 只有 MAC 有效时才添加对端（全零 MAC 会导致 assert 崩溃）
  if (macValid) {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, _peerMAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
#if DEBUG_SERIAL
      Serial.println("[ESP-NOW] 添加对端失败!");
      Serial.flush();
#endif
      _initialized = false;
      return false;
    }
#if DEBUG_SERIAL
    Serial.println("[ESP-NOW] 初始化完成，对端已添加");
    Serial.flush();
#endif
  } else {
#if DEBUG_SERIAL
    Serial.println("[ESP-NOW] 初始化完成（无对端，MAC 未配置）");
    Serial.flush();
#endif
  }

  _initialized = true;

#if DEBUG_SERIAL
  Serial.printf("[ESP-NOW] 初始化成功，剩余堆内存: %d bytes\n", ESP.getFreeHeap());
  Serial.flush();
#endif

  return true;
}

bool EspNowManager::_initWiFiLowLevel() {
#if DEBUG_SERIAL
  Serial.println("[ESP-NOW] 使用底层 API 初始化 WiFi...");
  Serial.flush();
#endif

  // 1. 初始化网络接口
  esp_err_t ret = esp_netif_init();
  if (ret != ESP_OK) {
#if DEBUG_SERIAL
    Serial.printf("[ESP-NOW] esp_netif_init 失败: %d\n", ret);
    Serial.flush();
#endif
    return false;
  }

  // 2. 创建默认事件循环
  ret = esp_event_loop_create_default();
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
#if DEBUG_SERIAL
    Serial.printf("[ESP-NOW] esp_event_loop_create_default 失败: %d\n", ret);
    Serial.flush();
#endif
    return false;
  }

  // 3. 创建默认 WiFi STA 网络接口
  esp_netif_create_default_wifi_sta();

  // 4. 初始化 WiFi
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ret = esp_wifi_init(&cfg);
  if (ret != ESP_OK) {
#if DEBUG_SERIAL
    Serial.printf("[ESP-NOW] esp_wifi_init 失败: %d\n", ret);
    Serial.flush();
#endif
    return false;
  }

  // 5. 设置 WiFi 模式为 STA
  ret = esp_wifi_set_mode(WIFI_MODE_STA);
  if (ret != ESP_OK) {
#if DEBUG_SERIAL
    Serial.printf("[ESP-NOW] esp_wifi_set_mode 失败: %d\n", ret);
    Serial.flush();
#endif
    return false;
  }

  // 6. 启动 WiFi
  ret = esp_wifi_start();
  if (ret != ESP_OK) {
#if DEBUG_SERIAL
    Serial.printf("[ESP-NOW] esp_wifi_start 失败: %d\n", ret);
    Serial.flush();
#endif
    return false;
  }

  // 7. 断开任何现有连接
  esp_wifi_disconnect();

#if DEBUG_SERIAL
  Serial.println("[ESP-NOW] WiFi 底层初始化成功");
  Serial.flush();
#endif

  return true;
}

void EspNowManager::startBatching() {
  if (!_batching) {
    _batchDelta = 0;
    _batchStart = millis();
    _batching = true;
  }
}

void EspNowManager::addRotation(int delta) {
  startBatching();   // 确保聚合窗口已开启
  _batchDelta += delta;
}

void EspNowManager::sendButtonEvent(PktEventType type, uint8_t batteryPct) {
  _sendPacket(type, 0, batteryPct);
}

bool EspNowManager::isWindowExpired() const {
  if (!_batching) return false;
  return (millis() - _batchStart) >= AGG_WINDOW_MS;
}

void EspNowManager::flush(uint8_t batteryPct) {
  if (!_batching || _batchDelta == 0) {
    _batching = false;
    return;
  }

  _sendPacket(PktEventType::ROTATE, _batchDelta, batteryPct);
  _batchDelta = 0;
  _batching = false;
}

bool EspNowManager::isBatching() const {
  return _batching;
}

bool EspNowManager::isActive() const {
  return _initialized;
}

// ============================================================
//  内部：发送数据包
// ============================================================

void EspNowManager::_sendPacket(PktEventType type, int delta, uint8_t batteryPct) {
  if (!_initialized) return;

  // 数据包结构 (8 字节):
  // [0] deviceID   [1] eventType  [2] delta(signed)  [3] battery%
  // [4-7] timestamp (millis, 低 4 字节)
  uint8_t pkt[PKT_SIZE];
  pkt[0] = PKT_DEVICE_ID;
  pkt[1] = static_cast<uint8_t>(type);
  pkt[2] = (int8_t)delta;   // 截断为 int8_t，单次聚合不会超过 ±127
  pkt[3] = batteryPct;

  uint32_t ts = millis();
  pkt[4] = (ts >> 0)  & 0xFF;
  pkt[5] = (ts >> 8)  & 0xFF;
  pkt[6] = (ts >> 16) & 0xFF;
  pkt[7] = (ts >> 24) & 0xFF;

  esp_err_t result = esp_now_send(_peerMAC, pkt, PKT_SIZE);

#if DEBUG_SERIAL
  if (result == ESP_OK) {
    Serial.printf("[ESP-NOW] 发送: type=0x%02X delta=%d bat=%d%%\n",
                  pkt[1], (int8_t)pkt[2], pkt[3]);
  } else {
    Serial.println("[ESP-NOW] 发送失败!");
  }
#endif
}

void EspNowManager::_onSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
#if DEBUG_SERIAL
  if (status != ESP_NOW_SEND_SUCCESS) {
    Serial.println("[ESP-NOW] 对端未确认");
  }
#endif
}
