#include "ui_connection_state.h"

/** Returns true until BLE reaches the connected state. */
bool shouldShowWaitingForConnection(BleState state) {
    return state != BleStateConnected;
}
