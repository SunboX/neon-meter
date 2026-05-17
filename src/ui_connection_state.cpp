#include "ui_connection_state.h"

/** Returns true until BLE or USB reaches a connected state. */
bool shouldShowWaitingForConnection(BleState state, bool usbConnected) {
    return state != BleStateConnected && !usbConnected;
}
