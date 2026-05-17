# Changelog

## 1.0.1 - 2026-05-17

### Added

- Added USB CDC serial support next to the existing BLE service.
- Added newline-delimited JSON control frames for USB host hello, firmware
  hello, host heartbeat, payload, acknowledgement, error, and refresh request.
- Added USB connection liveness tracking so the device detects when the host app
  closes, crashes, or stops sending heartbeat frames.
- Added UI connection state handling so the waiting screen clears when either
  BLE or USB is connected.

### Fixed

- Fixed USB protocol liveness expiry so the device returns to `Waiting for
  connection` after the USB host disappears.
- Preserved Clawdmeter-compatible payload fields while accepting wrapped
  provider bundles over USB and BLE.

### Documented

- Documented the combined USB/BLE protocol and the host-side DTR/RTS requirement
  discovered during USB debugging. Hosts must set DTR high so ESP32-S3 USB CDC
  delivers serial traffic, and should keep RTS low to avoid toggling the
  boot/reset line.

### Validation

- `npm test`
- `npm run build`
- Real-device USB handshake from the host app, including payload ack over the
  serial protocol.
