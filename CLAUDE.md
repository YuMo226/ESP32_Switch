# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ESP32-C3 "Smart Knob" — a low-power rotary encoder remote control for switching on a PC or controlling media. Communicates via **ESP-NOW** (primary) with 100ms batching, or **BLE HID** (fallback, activated on long press). Targets the **ESP32-C3 DevKitC-02** (RISC-V, single-core, Wi-Fi + BLE 5.0) using the **Arduino framework** via PlatformIO.

## Build Commands

| Action | Command |
|--------|---------|
| Build | `pio run` |
| Upload/Flash | `pio run --target upload` |
| Serial Monitor | `pio device monitor` |
| Build + Upload + Monitor | `pio run --target upload && pio device monitor` |
| Run Tests | `pio test` |
| Clean | `pio run --target clean` |

Serial port defaults to `/dev/ttyUSB0` at 115200 baud (configured in `platformio.ini`).

## Architecture

**Event-driven state machine** with strong module separation. The main loop does three things: poll encoder, poll button, update state machine. All business logic lives in `lib/` modules.

```
Hardware Layer (ISRs)
  encoder_manager (GPIO 5,6 — CHANGE interrupts, quadrature decode)
  button_manager  (GPIO 4 — CHANGE interrupt, debounce 50ms)
       |
       v  (set volatile flags; all logic runs in main-loop update())
  event_manager   (32-slot ring buffer, ISR-safe push)
       |
       v  (pop + dispatch)
  state_machine   (8 states, entry/exit hooks, timeout checks)
       |
       +---> esp_now_manager  (100ms batching, 8-byte packets)
       +---> ble_manager      (on-demand BLE HID, volume control)
       +---> sleep_manager    (Deep Sleep, GPIO wakeup, RTC memory)
       +---> power_manager    (CPU 40/80MHz scaling)
       +---> config_manager   (NVS persistence via storage_manager)
       +---> battery_manager  (ADC on GPIO 0, 16-sample averaging)
```

### State Machine States (`lib/state_machine/`)

`enum class State`: BOOT → IDLE → ACTIVE → ESPNOW_TX / BLE_MODE / CONFIG_MODE / LOW_BATTERY / DEEP_SLEEP

- **IDLE** (40MHz): any input → ACTIVE; 5s timeout → DEEP_SLEEP
- **ACTIVE** (80MHz): rotation → ESP-NOW batch; short press → ESP-NOW send; long press → BLE_MODE; 5s timeout → IDLE
- **BLE_MODE**: rotation → volume up/down; short press → mute; long press → IDLE; 60s idle → IDLE
- **DEEP_SLEEP**: saves encoder phase to RTC memory, enters Deep Sleep (does not return)

### Key Design Decisions

- **ISR separation**: ISRs in encoder/button managers only set flags; all logic runs in `update()` in the main loop
- **Low power**: CPU at 40MHz when idle, 80MHz when active; Deep Sleep after 5s inactivity (5-20µA); BLE only activated on demand
- **ESP-NOW batching**: rotation events aggregated over 100ms window, sent as single 8-byte packet; button events sent immediately
- **RTC memory**: encoder A/B phase persisted across Deep Sleep via `RTC_DATA_ATTR` for seamless wake
- **BLE lifecycle**: `BleKeyboard` object created on demand, deleted + `BLEDevice::deinit(true)` on exit to free RAM
- **Brownout disabled**: via direct register write (`RTC_CNTL_BROWN_OUT_REG`) and compile-time flags in `platformio.ini`

### Configuration (`include/config.h`)

All compile-time constants in one file, organized by section: GPIO pins, encoder params, power/timing thresholds, BLE name, ESP-NOW receiver MAC, battery thresholds, packet format, event queue size. The receiver MAC (`RECEIVER_MAC`) is currently all zeros — must be set to the actual receiver's MAC before ESP-NOW will work.

### Module Notes

- **event_manager**: `EventType` enum covers 20 event types across system, rotation, button, battery, BLE, ESP-NOW, and config categories
- **encoder_manager**: 20-pulse EC11 encoder, 4 ticks per detent, position clamped 0-100, 200µs ISR debounce
- **button_manager**: three-state FSM (IDLE → PRESSED → WAIT_DOUBLE), 1000ms long-press threshold, 300ms double-click window
- **esp_now_manager**: validates MAC on init (skips peer registration if all zeros)
- **config_manager**: `DeviceConfig` struct persisted as NVS blob under key "dev_cfg" in namespace "knob"
- **storage_manager**: handles NVS partition corruption by auto-erasing and reinitializing
- **battery_manager**: 3300mV (empty) to 4200mV (full), 10% low-battery threshold

## Key Files

- [src/main.cpp](src/main.cpp) — entry point, initialization sequence, main loop
- [include/config.h](include/config.h) — all compile-time constants
- [platformio.ini](platformio.ini) — board config, build flags, library deps
- [lib/state_machine/](lib/state_machine/) — central orchestrator, state transitions
- [lib/event_manager/](lib/event_manager/) — event queue (ring buffer)
