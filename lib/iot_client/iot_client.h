#ifndef IOT_CLIENT_H
#define IOT_CLIENT_H

#include <Arduino.h>

// ============================================================
//  IoT 远程控制模块 — Wi-Fi + MQTT
//
//  功能：连接互联网，通过 MQTT 接收远程控制命令，
//        上报设备状态。
//
//  依赖库（在 platformio.ini 中配置）:
//    knolleary/PubSubClient@^2.8
// ============================================================

class IoTClient {
public:
  /**
   * 初始化 Wi-Fi 和 MQTT 客户端
   * SSID/Password/Broker 地址在 global_config.h 中配置
   */
  void begin();

  /**
   * 维持 MQTT 连接和消息循环
   * 需要在 loop() 中持续调用
   */
  void loop();

  /**
   * 发布状态消息到 MQTT
   * @param payload  消息内容（JSON 字符串）
   */
  void publishStatus(const char* payload);

  /**
   * 检查是否已连接到 MQTT Broker
   */
  bool isConnected() const;

private:
  bool _wifiConnected = false;
  bool _mqttConnected = false;
  unsigned long _lastReconnectAttempt = 0;

  void _connectWiFi();
  void _connectMQTT();
  static void _onMQTTMessage(char* topic, uint8_t* payload, unsigned int length);
};

#endif // IOT_CLIENT_H
