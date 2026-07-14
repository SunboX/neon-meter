# Development Workflow

This page collects the local development and implementation details that are not
needed for the main README.

## Project Layout

- `src/` firmware source
- `docs/assets/` README photos and generated screenshots
- `docs/` protocol and development notes
- `specs/` implementation spec
- `tests/` native PlatformIO tests
- `web/esp-web-tools/` ESP Web Tools installer source

## Commands

The npm scripts prepend `~/.platformio/penv/bin` to `PATH` and then call
PlatformIO:

```bash
npm test
npm run build
npm run screenshots
npm run upload
npm run monitor
```

Equivalent direct PlatformIO commands:

```bash
~/.platformio/penv/bin/pio test -e native
~/.platformio/penv/bin/pio run -e m5stack-cores3
~/.platformio/penv/bin/pio run -e m5stack-cores3 -t upload
~/.platformio/penv/bin/pio device monitor -b 115200
```

If PlatformIO is installed elsewhere, update the `PATH` prefix in
`package.json` or use the full PlatformIO path for your local install.

## ESP Web Tools

The installer is built for the M5Stack CoreS3 ESP32-S3 target. Its standard
manifest flashes split bootloader, partition-table, OTA-selector, and app
images without including the NVS partition that stores the BLE identity.

For local development, the installer source lives in `web/esp-web-tools/`. On
every push to the `main` branch, `.github/workflows/deploy-web-tools.yml`
validates the installer metadata, builds the `m5stack-cores3` firmware, and
publishes all four split images plus `firmware.factory.bin`. The merged factory
image is retained only for an explicit full-erase recovery flow. Web Serial
requires the HTTPS GitHub Pages URL; opening the static file directly will not
work.

Validate the installer metadata with:

```bash
npm run validate:web-tools
```

## Screenshots

Rendered UI states are generated from the project theme and layout constants:

```bash
npm run screenshots
```

The generated SVG files are stored in `docs/assets/` and shown in the README.

## PlatformIO Notes

The CoreS3 environment uses `scripts/patch_m5gfx.py` during the PlatformIO
build. It excludes M5GFX's bundled LVGL font shim sources so M5Unified can drive
the hardware while the firmware links against the real LVGL library.
