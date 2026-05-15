#include <unity.h>

#include "ui_connection_state.h"

/** Verifies the waiting screen stays visible until BLE connects. */
void testWaitingScreenShowsUntilBleConnects(void) {
    TEST_ASSERT_TRUE(shouldShowWaitingForConnection(BleStateInit));
    TEST_ASSERT_TRUE(shouldShowWaitingForConnection(BleStateAdvertising));
    TEST_ASSERT_TRUE(shouldShowWaitingForConnection(BleStateDisconnected));
    TEST_ASSERT_FALSE(shouldShowWaitingForConnection(BleStateConnected));
}

/** Runs the UI connection-state native test suite. */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(testWaitingScreenShowsUntilBleConnects);
    return UNITY_END();
}
