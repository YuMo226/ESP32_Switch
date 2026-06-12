#include "iot_client.h"
#include "global_config.h"
#include <WiFi.h>

// ============================================================
//  注意：MQTT 功能需要 PubSubClient 库
//
//  请在 platformio.ini 的 lib_deps 中添加：
//    lib_deps = knolleary/PubSubClient@^2.8
//
//  目前为桩代码，仅实现 Wi-Fi 连接。
//  MQTT 部分在添加依赖后启用。
// ============================================================

// 暂时注释掉 MQTT 相关头文件，等添加依赖后取消注释
// #include <PubSubClient.h>
// static WiFiClient   wifiClient;
// static PubSubClient mqttClient(wifiClient);

// ============================================================
//  初始化
// ============================================================

void IoTClient::begin() {
  _connectWiFi();
  // TODO: _connectMQTT() 在添加 PubSubClient 依赖后启用
#if DEBUG_SERIAL
  Serial.println("[IoT] 初始化完成（MQTT 桩代码）");
#endif
}

// ============================================================
//  主循环 — 维持连接
// ============================================================

void IoTClient::loop() {
  if (!_wifiConnected) {
    // Wi-Fi 断开，尝试重连
    unsigned long now = millis();
    if (now - _lastReconnectAttempt > 10000) {   // 每 10 秒重试
      _lastReconnectAttempt = now;
      _connectWiFi();
    }
    return;
  }

  // TODO: mqttClient.loop() 在添加 PubSubClient 依赖后启用
}

// ============================================================
//  发布状态
// ============================================================

void IoTClient::publishStatus(const char* payload) {
  if (!_mqttConnected) return;
  // TODO: mqttClient.publish(MQTT_TOPIC_STATUS, payload);
#if DEBUG_SERIAL
  Serial.print("[IoT] 发布状态: ");
  Serial.println(payload);
#endif
}

bool IoTClient::isConnected() const {
  return _wifiConnected && _mqttConnected;
}

// ============================================================
//  内部实现
// ============================================================

void IoTClient::_connectWiFi() {
#if DEBUG_SERIAL
  Serial.print("[IoT] 正在连接 Wi-Fi: ");
  Serial.println(WIFI_SSID);
#endif

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // 等待连接，最多 10 秒
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    attempts++;
  }

  _wifiConnected = (WiFi.status() == WL_CONNECTED);

#if DEBUG_SERIAL
  if (_wifiConnected) {
    Serial.print("[IoT] Wi-Fi 已连接，IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[IoT] Wi-Fi 连接失败（将在后台重试）");
  }
#endif
}

void IoTClient::_connectMQTT() {
  // TODO: 在 platformio.ini 添加 PubSubClient 依赖后实现
  //
  // mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  // mqttClient.setCallback(_onMQTTMessage);
  //
  // if (mqttClient.connect("esp32-knob")) {
  //   mqttClient.subscribe(MQTT_TOPIC_CMD);
  //   _mqttConnected = true;
  // }
}

void IoTClient::_onMQTTMessage(char* topic, uint8_t* payload, unsigned int length) {
  // TODO: 解析远程命令，触发对应动作
  // payload 示例: {"cmd":"power_on"}
#if DEBUG_SERIAL
  Serial.print("[IoT] 收到消息 [");
  Serial.print(topic);
  Serial.print("]: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
#endif
}
