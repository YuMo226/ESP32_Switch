#include <Arduino.h>

// ============================================================
//  EC11 旋转编码器 — ESP32-C3 驱动示例
//  引脚:  A -> GPIO5,  B -> GPIO6,  Button -> GPIO4
// ============================================================

// ---------- 引脚定义 ----------
#define PIN_ENC_A    5   // 编码器 A 相（信号 A）
#define PIN_ENC_B    6   // 编码器 B 相（信号 B）
#define PIN_BUTTON   4   // 编码器内置按键（按下接地）

// ---------- 防抖时间（毫秒） ----------
#define ENC_DEBOUNCE_US   800    // 旋转防抖：800us（微秒）
#define BTN_DEBOUNCE_MS   50     // 按键防抖：50ms（毫秒）

// ---------- 全局变量 ----------
volatile int  encoderPos    = 0;          // 累计位置（顺时针 +1，逆时针 -1）
volatile int  lastDirection = 0;          // 最近一次方向：+1 顺时针，-1 逆时针
volatile bool encoderMoved  = false;      // 标记：旋转事件发生

volatile bool buttonPressed = false;      // 标记：按键按下事件
unsigned long lastBtnPress  = 0;          // 上次按键时间戳（防抖用）

// ============================================================
//  中断服务函数（ISR） — 必须用 IRAM_ATTR 放到 RAM 中执行
// ============================================================

/**
 * 编码器旋转中断（仅挂在 A 相上升沿）
 *
 * 原理：当 A 相出现上升沿时，读取 B 相电平
 *   - B == HIGH → 顺时针 (CW)
 *   - B == LOW  → 逆时针 (CCW)
 *
 * 这是最常用的"单边沿"解码方式，简单可靠。
 * 如果需要更高精度（4倍频），可以改为双边沿 + 查表法。
 */
void IRAM_ATTR encoderISR() {
  static unsigned long lastTime = 0;
  unsigned long now = micros();

  // 软件防抖：两次有效边沿间隔必须大于阈值
  if (now - lastTime < ENC_DEBOUNCE_US) return;
  lastTime = now;

  // 读取 B 相电平判断方向
  if (digitalRead(PIN_ENC_B) == HIGH) {
    encoderPos++;      // 顺时针
    lastDirection = 1;
  } else {
    encoderPos--;      // 逆时针
    lastDirection = -1;
  }
  encoderMoved = true;
}

/**
 * 按键中断（下降沿触发 — 按下瞬间）
 *
 * EC11 按键一端接 GPIO，另一端接 GND。
 * 未按下时为 HIGH（上拉），按下时变 LOW（接地）。
 * 因此用 FALLING 检测按下动作。
 */
void IRAM_ATTR buttonISR() {
  unsigned long now = millis();
  // 软件防抖
  if (now - lastBtnPress < BTN_DEBOUNCE_MS) return;
  lastBtnPress = now;

  buttonPressed = true;
}

// ============================================================
//  Arduino 入口
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(100);   // 等待串口稳定
  Serial.println();
  Serial.println("=== EC11 旋转编码器测试 ===");

  // ---- 配置 GPIO 输入模式 ----
  //
  // EC11 编码器内部是机械触点开关：
  //   - A、B 相：旋转时交替接通/断开到公共端（通常接 GND）
  //   - 按键：按下时接通到 GND
  //
  // 因此需要上拉电阻保持空闲状态为 HIGH。
  //   如果你的模块**板载了外部上拉电阻**（常见于成品模块），
  //   可以改为 INPUT 省电；否则必须用 INPUT_PULLUP。
  //   这里默认开启内部上拉，兼容有/无外部上拉的情况。

  pinMode(PIN_ENC_A,  INPUT);
  pinMode(PIN_ENC_B,  INPUT);
  pinMode(PIN_BUTTON, INPUT);

  // ---- 挂载中断 ----
  // A 相上升沿触发（旋转检测）
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_A), encoderISR, RISING);
  // 按键下降沿触发（按下检测）
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON), buttonISR, FALLING);

  Serial.println("就绪 — 旋转旋钮或按下按键查看输出");
  Serial.println("----------------------------------");
}

void loop() {
  // ---- 处理旋转事件 ----
  if (encoderMoved) {
    // 关中断读取共享变量，防止读到半更新的值
    noInterrupts();
    int pos = encoderPos;
    int dir = lastDirection;
    encoderMoved = false;
    interrupts();

    // 打印方向 + 当前位置
    Serial.print(dir > 0 ? "顺时针(CW) " : "逆时针(CCW) ");
    Serial.print("| Position = ");
    Serial.println(pos);
  }

  // ---- 处理按键事件 ----
  if (buttonPressed) {
    buttonPressed = false;
    Serial.println(">>> 按键按下 <<<");
  }

  // 主循环不需要 delay，CPU 会被中断唤醒处理
  // 如果想省电可以加 delay(1) 让出 CPU
}
