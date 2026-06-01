# Changelog

## 1.0.5 - 2026-06-01

### Fixed

- Treated provider windows with reset minutes at `0` as freshly reset usage
  instead of preserving stale exhausted percentages.

### Documented

- Clarified that `sr` and `wr` values of `0` mean the reset is due.

### Changed

- Updated firmware, protocol, and ESP Web Tools version metadata to 1.0.5.

### Validation

- `npm run validate:web-tools`
- `npm test`
- `npm run build`

## 1.0.4 - 2026-05-29

### Fixed

- Hid the shared battery indicator when CoreS3 hardware reports no attached
  battery.
- Corrected CoreS3 charging detection to use the M5Unified charging state enum.

### Changed

- Propagated battery attachment state through the UI update path.
- Updated firmware, protocol, and ESP Web Tools version metadata to 1.0.4.

### Validation

- `npm run validate:web-tools`
- `npm test`
- `npm run build`

## 1.0.2 - 2026-05-18

### Added

- Added firmware version metadata to USB hello frames and a read-only BLE
  metadata characteristic.
- Added an Info screen footer that shows the CoreS3 firmware version.

### Changed

- Switched usage gauges to display remaining capacity while preserving consumed
  usage fields for host compatibility.
- Updated the BLE service to use the Arduino NimBLE backend exposed through the
  ESP32 BLE compatibility API.
- Refined Info and usage panel spacing, backgrounds, and generated README
  screenshots.

### Validation

- `npm test`
- `npm run build`

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
