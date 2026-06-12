#include <Arduino.h>
#include "global_config.h"
#include "ec11_encoder.h"
#include "radio_manager.h"

// ESP-IDF 底层 API，用于禁用欠压检测
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ============================================================
//  无线远程开关机旋钮 — 主程序
//
//  职责：初始化各模块，读取旋钮事件，交给 radio_manager 路由
//  不关心数据走 BLE 还是 ESP-NOW，全部由 radio_manager 决策
// ============================================================

EC11Encoder  encoder;
RadioManager radio;

// 接收端 MAC 地址
static const uint8_t receiverMAC[6] = RECEIVER_MAC;

void setup() {
  // 禁用欠压检测器（防止 USB 供电不足时射频初始化导致重启循环）
  // TODO: 后续改善供电后移除此行
  REG_WRITE(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(SERIAL_BAUD);
  delay(100);

  Serial.println();
  Serial.println("========================================");
  Serial.println("  无线远程开关机旋钮 v0.2");
  Serial.println("  EC11 + BLE HID + ESP-NOW");
  Serial.println("========================================");

  // 1. 初始化旋钮（中断驱动，最先）
  encoder.begin();

  // 2. 初始化无线管理器（BLE + ESP-NOW）
  radio.begin(receiverMAC);

  Serial.println();
  Serial.println("系统就绪 — 旋转旋钮或按下按键");
  Serial.println("----------------------------------------");
}

void loop() {
  // ---- 读取旋钮事件 ----
  EC11Event evt = encoder.update();

  switch (evt) {
    case EC11Event::CW:
      Serial.print("顺时针(CW)  | 音量 = ");
      Serial.print(encoder.getPosition());
      Serial.println("%");
      radio.handleLocalVolume(+1);
      break;

    case EC11Event::CCW:
      Serial.print("逆时针(CCW) | 音量 = ");
      Serial.print(encoder.getPosition());
      Serial.println("%");
      radio.handleLocalVolume(-1);
      break;

    case EC11Event::PRESSED:
      Serial.println(">>> 按键按下 <<<");
      radio.handleLocalPowerToggle();
      break;

    case EC11Event::NONE:
    default:
      break;
  }

  // ---- 维持无线协议栈 ----
  radio.loop();
}
