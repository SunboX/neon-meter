#pragma once

#include "ble_service.h"

/**
 * Returns true while the UI should block on the waiting-for-connection screen.
 */
bool shouldShowWaitingForConnection(BleState state, bool usbConnected);
