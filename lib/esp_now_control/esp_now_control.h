#ifndef ESP_NOW_CONTROL_H
#define ESP_NOW_CONTROL_H

#include <Arduino.h>
#include <esp_now.h>

// ============================================================
//  ESP-NOW 本地无线通信模块
//
//  功能：与接收端建立低延迟点对点连接，
//        发送开关机和音量控制命令。
//
//  命令协议（1 字节）:
//    0x01 = 开机 (POWER_ON)
//    0x02 = 关机 (POWER_OFF)
//    0x03 = 音量增加 (VOL_UP)
//    0x04 = 音量减少 (VOL_DOWN)
//    0x05 = 静音 (MUTE)
// ============================================================

// 控制命令定义
enum class Cmd : uint8_t {
  POWER_ON    = 0x01,
  POWER_OFF   = 0x02,
  VOL_UP      = 0x03,
  VOL_DOWN    = 0x04,
  MUTE        = 0x05,
};

class ESPNowControl {
public:
  /**
   * 初始化 ESP-NOW，添加接收端为对等设备
   * @param peerMAC  接收端的 MAC 地址（6 字节）
   * @return true = 成功
   */
  bool begin(const uint8_t* peerMAC);

  /**
   * 发送一条控制命令
   * @param cmd  命令枚举值
   * @return true = 发送成功
   */
  bool sendCommand(Cmd cmd);

private:
  uint8_t _peerMAC[6] = {};
  bool    _initialized = false;

  // 发送完成回调
  static void _onSent(const uint8_t* mac_addr, esp_now_send_status_t status);
};

#endif // ESP_NOW_CONTROL_H
