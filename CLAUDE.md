# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ESP32-based project for remotely switching/turning on a PC. Targets the **ESP32-C3 DevKitC-02** board (RISC-V, single-core, Wi-Fi + BLE 5.0) using the **Arduino framework** via PlatformIO.

## Build Commands

| Action | Command |
|--------|---------|
| Build | `pio run` |
| Upload/Flash | `pio run --target upload` |
| Serial Monitor | `pio device monitor` |
| Build + Upload + Monitor | `pio run --target upload && pio device monitor` |
| Run Tests | `pio test` |
| Clean | `pio run --target clean` |

## Architecture

- **Framework:** Arduino on ESP32-C3 (espressif32 platform)
- **Entry point:** [src/main.cpp](src/main.cpp) — standard Arduino `setup()` / `loop()` pattern
- **Headers:** [include/](include/) — project-wide header files
- **Libraries:** [lib/](lib/) — local library dependencies
- **Config:** [platformio.ini](platformio.ini) — board, framework, and build settings
