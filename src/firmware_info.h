#pragma once

#include "ble_service.h"

#include <stddef.h>
#include <stdio.h>

#ifndef NEON_METER_FIRMWARE_VERSION
#define NEON_METER_FIRMWARE_VERSION "1.0.3"
#endif

/** Current firmware semantic version string. */
constexpr const char *kFirmwareVersion = NEON_METER_FIRMWARE_VERSION;

/** Firmware target chip family used by ESP Web Tools manifests. */
constexpr const char *kFirmwareChipFamily = "ESP32-S3";

/** Title shown on the BLE information screen. */
constexpr const char *kInfoScreenTitle = "Info";

/** Status label shown while BLE advertising is active. */
constexpr const char *kBleAdvertisingStatusText = "BLE Advertising";

/** Footer label shown at the bottom of the info screen. */
constexpr const char *kInfoFooterText = "Neon Meter CoreS3 v" NEON_METER_FIRMWARE_VERSION;

/** Title shown on the privacy screensaver overlay. */
constexpr const char *kScreensaverTitleText = "NEON METER";

/** Privacy notice shown on the screensaver overlay. */
constexpr const char *kScreensaverPrivacyText = "PRIVACY MODE";

/** Static hardware metadata shown on the screensaver overlay. */
constexpr const char *kScreensaverHardwareText = "CORE S3 / ESP32-S3";

/** Static hidden-usage notice shown on the screensaver overlay. */
constexpr const char *kScreensaverHiddenText = "USAGE HIDDEN";

/** Returns privacy-safe transport status text for the screensaver overlay. */
inline const char *screensaverConnectionText(BleState state, bool usbConnected) {
    if (usbConnected) return "LINK USB SERIAL";
    switch (state) {
    case BleStateConnected:
        return "LINK BLE CONNECTED";
    case BleStateAdvertising:
        return "LINK BLE ADVERTISING";
    case BleStateDisconnected:
        return "LINK BLE OFFLINE";
    default:
        return "LINK STARTING";
    }
}

/** Formats non-secret firmware metadata as compact JSON. */
inline void formatFirmwareMetadata(char *buffer, size_t bufferLength) {
    if (!buffer || bufferLength == 0) return;
    snprintf(buffer, bufferLength,
             "{\"firmwareVersion\":\"%s\",\"chipFamily\":\"%s\"}",
             kFirmwareVersion, kFirmwareChipFamily);
}
