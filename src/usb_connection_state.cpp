#include "usb_connection_state.h"

/**
 * Returns true while the USB serial host app is considered active.
 */
bool isUsbProtocolConnected(const UsbConnectionState *state) {
    return state && state->connected;
}

/**
 * Records one inbound USB protocol frame from the host app.
 */
void noteUsbProtocolActivity(UsbConnectionState *state, uint32_t nowMs) {
    if (!state) return;
    state->connected = true;
    state->lastSeenMs = nowMs;
}

/**
 * Clears USB activity after the host app stops sending frames.
 */
bool expireUsbProtocolIfInactive(UsbConnectionState *state, uint32_t nowMs) {
    if (!state || !state->connected) return false;
    if (nowMs - state->lastSeenMs <= kUsbProtocolTimeoutMs) return false;
    state->connected = false;
    return true;
}
