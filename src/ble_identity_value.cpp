#include "ble_identity_value.h"

/** Normalizes the type bits and avoids reserved random-part values. */
bool normalizeBleRandomStaticAddress(uint8_t *address, size_t length) {
    if (address == nullptr || length != kBleIdentityAddressSize) return false;

    address[5] = static_cast<uint8_t>((address[5] & 0x3F) | 0xC0);

    bool allZeros = (address[5] & 0x3F) == 0x00;
    bool allOnes = (address[5] & 0x3F) == 0x3F;
    for (size_t index = 0; index < length - 1; ++index) {
        allZeros = allZeros && address[index] == 0x00;
        allOnes = allOnes && address[index] == 0xFF;
    }
    if (allZeros) address[0] = 0x01;
    if (allOnes) address[0] = 0xFE;
    return true;
}

/** Validates the type bits and excludes reserved random-part values. */
bool isBleRandomStaticAddress(const uint8_t *address, size_t length) {
    if (address == nullptr || length != kBleIdentityAddressSize) return false;
    if ((address[5] & 0xC0) != 0xC0) return false;

    bool allZeros = (address[5] & 0x3F) == 0x00;
    bool allOnes = (address[5] & 0x3F) == 0x3F;
    for (size_t index = 0; index < length - 1; ++index) {
        allZeros = allZeros && address[index] == 0x00;
        allOnes = allOnes && address[index] == 0xFF;
    }
    return !allZeros && !allOnes;
}
