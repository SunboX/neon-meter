# AGENTS

## Project Overview

- Repository: Neon Meter firmware.
- Purpose: PlatformIO Arduino/LVGL firmware for the M5Stack CoreS3 ESP32-S3 IoT Development Kit.
- Source is in `src/`.
- Tests are in `tests/`.
- Documentation is in `docs/`.
- Specs are in `specs/`.

## Build, Run, Test

- Native tests: `npm test`
- Firmware build: `npm run build`
- Upload: `npm run upload`
- Monitor: `npm run monitor`

The npm scripts prepend `~/.platformio/penv/bin` to PATH before invoking PlatformIO.

## Firmware Architecture Rules

- Keep UI rendering in `src/ui.*` and splash behavior in `src/splash.*`.
- Keep BLE service behavior in `src/ble_service.*`.
- Keep provider payload parsing and clamping in `src/usage_model.*`.
- Keep M5Unified/M5GFX includes isolated in `src/m5_hal.*` so they do not conflict with real LVGL headers.
- Do not add Claude crab/Clawd artwork, icons, fonts, or proprietary brand assets.

## Protocol Rules

- Keep BLE UUIDs synchronized with `docs/protocol.md` and the sibling `Neon-Meter-Host` project.
- Preserve compact Clawdmeter-compatible fields: `s`, `sr`, `w`, `wr`, `st`, and `ok`.
- Preserve provider metadata fields: `p`, `title`, `sl`, `wl`, and `detail`.
- Do not put OpenAI or Anthropic API keys on the device.

## Coding Style

- Use focused C++ modules and headers under `src/`.
- Use Arduino-style names in code: PascalCase for types/classes/enums, lowerCamelCase for variables/functions/methods, and `kLowerCamelCase` for constants.
- Always add concise documentation for every class, struct, enum, function, and method declaration/definition, plus short inline documentation where implementation logic is not self-explanatory.
- Keep comments short and only where the code is not self-explanatory.
- Add or update native tests for payload parsing, clamping, and rate logic.
- Before claiming completion, run `npm test` and `npm run build`.
