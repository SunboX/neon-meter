#pragma once

/** Loads or creates the persistent BLE identity and applies it to NimBLE. */
bool configurePersistentBleIdentity();

/** Replaces the stored BLE identity so the next boot advertises as a new device. */
bool rotatePersistentBleIdentity();
