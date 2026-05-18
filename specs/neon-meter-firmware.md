# Neon Meter Firmware Spec

## Goal

Build a PlatformIO Arduino firmware project for the M5Stack CoreS3 ESP32-S3 IoT
Development Kit. The device shows AI service usage in a compact LVGL dashboard
inspired by Clawdmeter, while avoiding Claude-specific mascot, icon, font, and
brand assets.

## Hardware Target

- Board: M5Stack CoreS3, ESP32-S3, 320x240 touch display, 16 MB flash, 8 MB
  PSRAM.
- Framework: Arduino through PlatformIO.
- Display stack: M5Unified/M5GFX for display, touch, battery, and button
  access.
- UI stack: LVGL 9.
- Transport: USB CDC serial and BLE GATT service using the Arduino BLE NimBLE
  backend.

## User Experience

- The default screen is a dark usage dashboard with:
  - small neutral Neon Meter mark at top left,
  - title,
  - battery indicator,
  - two usage panels with remaining percent, label, progress bar, and reset/period detail,
  - small data-backed status line with an animated dot loader at the bottom.
- A Bluetooth screen shows connection state, advertised device name, BLE
  address, and a tap zone for clearing bonds.
- Firmware version metadata is exposed to the host over USB hello frames and a
  read-only BLE metadata characteristic so the host can offer updates.
- A splash screen uses a neutral neon sweep animation instead of Claude
  crab/Clawd art or block-letter branding.
- Touching the usage screen toggles the splash. Touching the splash returns to
  the last non-splash screen.
- Pressing the CoreS3 power button cycles Usage and Bluetooth screens when not
  on splash, and advances the splash animation when on splash.

## Provider Model

The firmware accepts provider-neutral JSON over BLE. It remains compatible with
the compact Clawdmeter payload shape:

```json
{ "s": 45, "sr": 120, "w": 28, "wr": 7200, "st": "allowed", "ok": true }
```

Current hosts can also send a provider bundle:

```json
{
  "rotationSeconds": 30,
  "providers": [
    {
      "p": "claude",
      "title": "Claude Code",
      "s": 45,
      "sl": "Session",
      "sr": 120,
      "w": 28,
      "wl": "Weekly",
      "wr": 7200,
      "st": "allowed",
      "detail": "5h 45% / 7d 28%",
      "ok": true
    },
    {
      "p": "chatgpt",
      "title": "ChatGPT",
      "s": 12,
      "sl": "Session",
      "sr": 240,
      "w": 8,
      "wl": "Weekly",
      "wr": 6800,
      "st": "ok",
      "detail": "5h 12% / 7d 8%",
      "ok": true
    }
  ]
}
```

If the bundle contains one provider, the device keeps showing it. If the bundle
contains two providers, the device rotates between cached provider screens using
`rotationSeconds`, defaulting to 30 seconds.

The firmware does not store credentials or call provider APIs directly. A host
daemon should fetch usage and map it to this payload. This keeps credentials off
the microcontroller.

## Non-Goals

- No Claude crab/Clawd animation, Claude icon, Anthropic fonts, or proprietary
  assets.
- No device-side credential entry.
- No BLE HID keyboard shortcuts in the first implementation.
