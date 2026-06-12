#include <Arduino.h>
#include "global_config.h"
#include "ec11_encoder.h"
// #include "esp_now_control.h"    // TODO: 后续启用
// #include "iot_client.h"          // TODO: 后续启用

// ============================================================
//  无线远程开关机旋钮 — 主程序
//  当前阶段：仅调试 EC11 旋钮模块
// ============================================================

EC11Encoder encoder;
// ESPNowControl espNow;            // TODO: 后续启用
// IoTClient     iotClient;         // TODO: 后续启用

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(100);

  Serial.println();
  Serial.println("========================================");
  Serial.println("  无线远程开关机旋钮 v0.1");
  Serial.println("  [阶段1] EC11 旋钮调试");
  Serial.println("========================================");

  encoder.begin();

  // espNow.begin(receiverMAC);      // TODO: 后续启用
  // iotClient.begin();              // TODO: 后续启用

  Serial.println();
  Serial.println("系统就绪 — 旋转旋钮或按下按键查看输出");
  Serial.println("----------------------------------------");
}

void loop() {
  EC11Event evt = encoder.update();

  switch (evt) {
    case EC11Event::CW:
      Serial.print("顺时针(CW)  | 音量 = ");
      Serial.print(encoder.getPosition());
      Serial.println("%");
      // espNow.sendCommand(Cmd::VOL_UP);        // TODO
      break;

    case EC11Event::CCW:
      Serial.print("逆时针(CCW) | 音量 = ");
      Serial.print(encoder.getPosition());
      Serial.println("%");
      // espNow.sendCommand(Cmd::VOL_DOWN);      // TODO
      break;

    case EC11Event::PRESSED:
      Serial.println(">>> 按键按下 <<<");
      // espNow.sendCommand(Cmd::POWER_ON);       // TODO
      break;

    case EC11Event::NONE:
    default:
      break;
  }

  // iotClient.loop();                           // TODO: 后续启用
}
