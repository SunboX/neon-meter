#include <unity.h>

#include "usb_connection_state.h"

/** Verifies USB liveness starts disconnected and becomes active after a host frame. */
void testUsbConnectionBecomesActiveAfterProtocolFrame(void) {
    UsbConnectionState state = {};

    TEST_ASSERT_FALSE(isUsbProtocolConnected(&state));

    noteUsbProtocolActivity(&state, 1000);

    TEST_ASSERT_TRUE(isUsbProtocolConnected(&state));
}

/** Verifies USB liveness expires after the host stops sending frames. */
void testUsbConnectionExpiresAfterHeartbeatTimeout(void) {
    UsbConnectionState state = {};

    noteUsbProtocolActivity(&state, 1000);

    TEST_ASSERT_FALSE(expireUsbProtocolIfInactive(&state, 1000 + kUsbProtocolTimeoutMs));
    TEST_ASSERT_TRUE(isUsbProtocolConnected(&state));
    TEST_ASSERT_TRUE(expireUsbProtocolIfInactive(&state, 1001 + kUsbProtocolTimeoutMs));
    TEST_ASSERT_FALSE(isUsbProtocolConnected(&state));
}

/** Verifies inactive USB state does not report repeated timeout changes. */
void testUsbConnectionTimeoutReportsChangeOnlyOnce(void) {
    UsbConnectionState state = {};

    TEST_ASSERT_FALSE(expireUsbProtocolIfInactive(&state, 999999));
}

/** Runs the USB connection state native test suite. */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(testUsbConnectionBecomesActiveAfterProtocolFrame);
    RUN_TEST(testUsbConnectionExpiresAfterHeartbeatTimeout);
    RUN_TEST(testUsbConnectionTimeoutReportsChangeOnlyOnce);
    return UNITY_END();
}
