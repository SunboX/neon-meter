#pragma once

#include <stdint.h>

/**
 * Connection states reported by the BLE service.
 */
enum BleState {
    BleStateInit = 0,
    BleStateAdvertising,
    BleStateConnected,
    BleStateDisconnected
};

/**
 * Starts the BLE server, characteristics, and advertising.
 */
void bleInit();

/**
 * Restarts advertising when a disconnect requested it.
 */
void bleTick();

/**
 * Returns the current BLE connection state.
 */
BleState getBleState();

/**
 * Returns the advertised BLE device name.
 */
const char *getBleDeviceName();

/**
 * Returns the upper-case BLE MAC address text shown on screen.
 */
const char *getBleAddress();

/**
 * Deletes paired device bonds and forces the central to reconnect.
 */
void clearBleBonds();

/**
 * Returns true when a complete provider payload is waiting.
 */
bool bleHasData();

/**
 * Consumes and returns the latest provider payload buffer.
 */
const char *readBleData();

/**
 * Notifies the connected host that a payload was accepted.
 */
void sendBleAck();

/**
 * Notifies the connected host that a payload was rejected.
 */
void sendBleNack();

/**
 * Asks the connected host to send a fresh provider payload.
 */
void requestBleRefresh();

/**
 * Returns the BLE firmware metadata characteristic UUID.
 */
const char *getBleMetadataUuid();
