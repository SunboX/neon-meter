#include <unity.h>

#include "ble_identity_value.h"

/** Verifies normalization marks an address as random static. */
void testNormalizeMarksAddressAsRandomStatic(void) {
    uint8_t address[] = {0x10, 0x20, 0x30, 0x40, 0x50, 0x01};

    TEST_ASSERT_TRUE(normalizeBleRandomStaticAddress(address, sizeof(address)));
    TEST_ASSERT_EQUAL_HEX8(0xC1, address[5]);
    TEST_ASSERT_TRUE(isBleRandomStaticAddress(address, sizeof(address)));
}

/** Verifies addresses with the wrong byte count are rejected. */
void testNormalizeRejectsWrongLength(void) {
    uint8_t address[] = {0x10, 0x20, 0x30, 0x40, 0x50};

    TEST_ASSERT_FALSE(normalizeBleRandomStaticAddress(address, sizeof(address)));
    TEST_ASSERT_FALSE(isBleRandomStaticAddress(address, sizeof(address)));
}

/** Verifies normalization avoids the reserved all-ones random-static value. */
void testNormalizeAdjustsReservedAllOnesAddress(void) {
    uint8_t address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    TEST_ASSERT_TRUE(normalizeBleRandomStaticAddress(address, sizeof(address)));
    TEST_ASSERT_NOT_EQUAL_HEX8(0xFF, address[0]);
    TEST_ASSERT_TRUE(isBleRandomStaticAddress(address, sizeof(address)));
}

/** Verifies normalization avoids the reserved all-zero random portion. */
void testNormalizeAdjustsReservedAllZerosRandomPart(void) {
    uint8_t address[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    TEST_ASSERT_TRUE(normalizeBleRandomStaticAddress(address, sizeof(address)));
    TEST_ASSERT_NOT_EQUAL_HEX8(0x00, address[0]);
    TEST_ASSERT_TRUE(isBleRandomStaticAddress(address, sizeof(address)));
}

/** Runs the BLE identity value native test suite. */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(testNormalizeMarksAddressAsRandomStatic);
    RUN_TEST(testNormalizeRejectsWrongLength);
    RUN_TEST(testNormalizeAdjustsReservedAllOnesAddress);
    RUN_TEST(testNormalizeAdjustsReservedAllZerosRandomPart);
    return UNITY_END();
}
