#pragma once

#include <stdint.h>

static constexpr uint32_t kUsbProtocolTimeoutMs = 15000;

/**
 * Tracks whether a USB serial host app is actively speaking the protocol.
 */
struct UsbConnectionState {
    bool connected;
    uint32_t lastSeenMs;
};

/**
 * Returns true while the USB serial host app is considered active.
 */
bool isUsbProtocolConnected(const UsbConnectionState *state);

/**
 * Records one inbound USB protocol frame from the host app.
 */
void noteUsbProtocolActivity(UsbConnectionState *state, uint32_t nowMs);

/**
 * Clears USB activity after the host app stops sending frames.
 */
bool expireUsbProtocolIfInactive(UsbConnectionState *state, uint32_t nowMs);
