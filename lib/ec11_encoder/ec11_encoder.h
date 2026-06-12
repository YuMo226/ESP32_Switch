#ifndef EC11_ENCODER_H
#define EC11_ENCODER_H

#include <Arduino.h>

// ============================================================
//  EC11 旋转编码器驱动 — 状态机查表法（四倍频）
//
//  使用方式:
//    EC11Encoder encoder;
//    encoder.begin();
//    // 在 loop() 中:
//    EC11Event evt = encoder.update();
//    if (evt == EC11Event::CW)  { ... }
// ============================================================

// 旋钮事件类型
enum class EC11Event : uint8_t {
  NONE      = 0,    // 无事件
  CW        = 1,    // 顺时针旋转
  CCW       = 2,    // 逆时针旋转
  PRESSED   = 3,    // 按键按下
};

class EC11Encoder {
public:
  /**
   * 初始化 GPIO、中断
   * 引脚定义在 global_config.h 中
   */
  void begin();

  /**
   * 在 loop() 中调用，检查并返回最新事件
   * 非阻塞，无事件时立即返回 NONE
   */
  EC11Event update();

  /**
   * 获取当前位置值（0 ~ 100，对应音量 0% ~ 100%）
   */
  int getPosition() const;

  /**
   * 设置位置值（自动钳位到 0~100）
   */
  void setPosition(int pos);

  /**
   * 重置位置计数为 0
   */
  void resetPosition();

private:
  // ---- 以下变量由中断访问，必须 volatile ----
  volatile int  _pos          = 0;
  volatile int  _lastDir      = 0;
  volatile bool _encMoved     = false;
  volatile bool _btnPressed   = false;
  unsigned long _lastBtnPress = 0;

  // 四倍频分频计数器：每 4 次状态跳变 = 1 个物理咔嗒
  volatile int  _tickCount    = 0;

  static const int POS_MIN = 0;
  static const int POS_MAX = 100;

  // 状态机
  uint8_t _prevEncState = 0;

  // ---- 中断服务函数（静态，通过指针访问实例成员） ----
  static void IRAM_ATTR _encoderISR(void* arg);
  static void IRAM_ATTR _buttonISR(void* arg);

  // 内部辅助
  uint8_t _readAB();
};

#endif // EC11_ENCODER_H
