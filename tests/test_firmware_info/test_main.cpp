#include <unity.h>

#include "firmware_info.h"

/** Verifies the info screen title uses the updated view name. */
void testInfoScreenUsesUpdatedTitle(void) {
    TEST_ASSERT_EQUAL_STRING("Info", kInfoScreenTitle);
}

/** Verifies the advertising BLE state uses the expanded status label. */
void testInfoScreenUsesBleAdvertisingStatus(void) {
    TEST_ASSERT_EQUAL_STRING("BLE Advertising", kBleAdvertisingStatusText);
}

/** Verifies the bottom info label exposes the current firmware version. */
void testInfoScreenFooterShowsFirmwareVersion(void) {
    TEST_ASSERT_EQUAL_STRING("Neon Meter CoreS3 v1.0.5", kInfoFooterText);
}

/** Verifies firmware metadata can be exposed through USB and BLE APIs. */
void testFirmwareMetadataFormatsJson(void) {
    char buffer[128] = {};

    formatFirmwareMetadata(buffer, sizeof(buffer));

    TEST_ASSERT_EQUAL_STRING(
        "{\"firmwareVersion\":\"1.0.5\",\"chipFamily\":\"ESP32-S3\"}",
        buffer);
}

/** Verifies screensaver connection text never includes provider usage data. */
void testScreensaverConnectionTextUsesOnlyTransportStatus(void) {
    TEST_ASSERT_EQUAL_STRING("LINK USB SERIAL", screensaverConnectionText(BleStateConnected, true));
    TEST_ASSERT_EQUAL_STRING("LINK BLE CONNECTED", screensaverConnectionText(BleStateConnected, false));
    TEST_ASSERT_EQUAL_STRING("LINK BLE ADVERTISING", screensaverConnectionText(BleStateAdvertising, false));
    TEST_ASSERT_EQUAL_STRING("LINK BLE OFFLINE", screensaverConnectionText(BleStateDisconnected, false));
    TEST_ASSERT_EQUAL_STRING("LINK STARTING", screensaverConnectionText(BleStateInit, false));
}

/** Runs the firmware info native test suite. */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(testInfoScreenUsesUpdatedTitle);
    RUN_TEST(testInfoScreenUsesBleAdvertisingStatus);
    RUN_TEST(testInfoScreenFooterShowsFirmwareVersion);
    RUN_TEST(testFirmwareMetadataFormatsJson);
    RUN_TEST(testScreensaverConnectionTextUsesOnlyTransportStatus);
    return UNITY_END();
}
