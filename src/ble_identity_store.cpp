#include "ble_identity_store.h"

#include "ble_identity_value.h"

#include <BLEDevice.h>
#include <Preferences.h>
#include <esp_random.h>

static constexpr const char *kBlePreferencesNamespace = "neon_ble";
static constexpr const char *kBleIdentityKey = "identity";

/** Generates and persists a fresh random-static identity. */
static bool writeNewIdentity(uint8_t *address) {
    esp_fill_random(address, kBleIdentityAddressSize);
    if (!normalizeBleRandomStaticAddress(address, kBleIdentityAddressSize)) return false;

    Preferences preferences;
    if (!preferences.begin(kBlePreferencesNamespace, false)) return false;
    size_t written = preferences.putBytes(kBleIdentityKey, address, kBleIdentityAddressSize);
    preferences.end();
    return written == kBleIdentityAddressSize;
}

/** Loads the stored identity when it is present and valid. */
static bool readStoredIdentity(uint8_t *address) {
    Preferences preferences;
    if (!preferences.begin(kBlePreferencesNamespace, true)) return false;
    size_t storedLength = preferences.getBytesLength(kBleIdentityKey);
    size_t readLength = storedLength == kBleIdentityAddressSize
                            ? preferences.getBytes(kBleIdentityKey, address, kBleIdentityAddressSize)
                            : 0;
    preferences.end();
    return readLength == kBleIdentityAddressSize &&
           isBleRandomStaticAddress(address, kBleIdentityAddressSize);
}

/** Loads or creates the persistent identity and configures NimBLE to use it. */
bool configurePersistentBleIdentity() {
    uint8_t address[kBleIdentityAddressSize] = {};
    if (!readStoredIdentity(address) && !writeNewIdentity(address)) return false;
    if (!BLEDevice::setOwnAddr(address)) return false;
    return BLEDevice::setOwnAddrType(BLE_OWN_ADDR_RANDOM);
}

/** Writes a fresh identity for the next firmware boot. */
bool rotatePersistentBleIdentity() {
    uint8_t address[kBleIdentityAddressSize] = {};
    return writeNewIdentity(address);
}
