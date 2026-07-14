# Changelog

## 1.0.7 - 2026-07-14

### Added

- Added a persistent random-static BLE identity that survives routine firmware
  updates and changes automatically after a full flash erase.
- Added the optional USB `ble-repair` capability, which clears device bonds,
  rotates the BLE identity, and restarts after acknowledging the host.
- Published split bootloader, partition-table, OTA-selector, and app images for
  non-erasing updates while retaining a separate merged factory image.

### Fixed

- Recovered automatically from one-sided stale BLE pairing state left behind
  when the device was erased but the computer retained its old bond.

### Changed

- Updated firmware, protocol, and ESP Web Tools version metadata to 1.0.7.
- Documented the safe update and explicit factory-recovery artifact roles.

### Validation

- `npm run validate:web-tools`
- `npm test`
- `npm run build`

## 1.0.6 - 2026-07-14

### Fixed

- Hide the complete Session usage panel when the host reports that ChatGPT's
  Session limit is unavailable, and promote Weekly usage to the upper slot.
- Restore the Session panel automatically when a later payload reports it as
  available again.
- Track the visible Weekly usage for activity and screensaver behavior while
  Session is unavailable.

### Changed

- Added backward-compatible support for the optional `se` payload capability;
  payloads without it retain the legacy two-panel display.
- Updated firmware, protocol, and ESP Web Tools version metadata to 1.0.6.

### Validation

- `npm run validate:web-tools`
- `npm test`
- `npm run build`

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
