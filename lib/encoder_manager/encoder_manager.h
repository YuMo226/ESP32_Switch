#ifndef ENCODER_MANAGER_H
#define ENCODER_MANAGER_H

#include <Arduino.h>
#include "config.h"

class EventManager;   // 前向声明，避免循环依赖

// ============================================================
//  编码器管理器 — EC11 旋转编码器
//
//  状态机查表法（四倍频） + 4 倍分频（20脉冲 EC11）
//  Deep Sleep 前后支持相位保存/恢复
// ============================================================

class EncoderManager {
public:
  /**
   * 初始化 GPIO + 中断，绑定事件管理器
   */
  void begin(EventManager* em);

  /**
   * 在 loop() 中调用，检查中断产生的事件并推入队列
   */
  void update();

  /**
   * 获取当前位置 (0-100)
   */
  int getPosition() const;

  /**
   * 保存当前 A/B 相位到 RTC 内存（Deep Sleep 前调用）
   */
  uint8_t getState() const;

  /**
   * 从 RTC 内存恢复相位（唤醒后调用）
   * @param savedState  之前保存的相位
   */
  void restoreState(uint8_t savedState);

private:
  EventManager* _em = nullptr;

  // ---- 中断共享变量 ----
  volatile int  _pos        = 0;
  volatile int  _lastDir    = 0;
  volatile bool _encMoved   = false;
  volatile int  _tickCount  = 0;

  // 状态机
  uint8_t _prevEncState = 0;

  // 内部方法
  uint8_t _readAB();

  // 中断服务函数
  static void IRAM_ATTR _encoderISR(void* arg);
};

#endif // ENCODER_MANAGER_H
