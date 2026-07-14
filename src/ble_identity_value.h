#pragma once

#include <stddef.h>
#include <stdint.h>

/** Number of bytes in a Bluetooth device address. */
static constexpr size_t kBleIdentityAddressSize = 6;

/** Normalizes a six-byte address into a valid BLE random-static identity. */
bool normalizeBleRandomStaticAddress(uint8_t *address, size_t length);

/** Returns whether an address is a valid BLE random-static identity. */
bool isBleRandomStaticAddress(const uint8_t *address, size_t length);
