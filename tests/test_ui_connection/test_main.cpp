#include <unity.h>

#include "ui_connection_state.h"

/** Verifies the waiting screen stays visible until BLE connects. */
void testWaitingScreenShowsUntilBleConnects(void) {
    TEST_ASSERT_TRUE(shouldShowWaitingForConnection(BleStateInit, false));
    TEST_ASSERT_TRUE(shouldShowWaitingForConnection(BleStateAdvertising, false));
    TEST_ASSERT_TRUE(shouldShowWaitingForConnection(BleStateDisconnected, false));
    TEST_ASSERT_FALSE(shouldShowWaitingForConnection(BleStateConnected, false));
    TEST_ASSERT_FALSE(shouldShowWaitingForConnection(BleStateAdvertising, true));
}

/** Runs the UI connection-state native test suite. */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(testWaitingScreenShowsUntilBleConnects);
    return UNITY_END();
}
