#include <unity.h>

#include "ui_gauge.h"

/** Verifies provider usage displays as remaining gauge capacity. */
void testUsageGaugeShowsRemainingPercent(void) {
    TEST_ASSERT_EQUAL(100, UiGauge::remainingPercent(0.0f));
    TEST_ASSERT_EQUAL(71, UiGauge::remainingPercent(29.0f));
    TEST_ASSERT_EQUAL(1, UiGauge::remainingPercent(99.0f));
    TEST_ASSERT_EQUAL(0, UiGauge::remainingPercent(100.0f));
}

/** Verifies remaining gauge capacity stays inside display bounds. */
void testUsageGaugeClampsRemainingPercent(void) {
    TEST_ASSERT_EQUAL(100, UiGauge::remainingPercent(-12.0f));
    TEST_ASSERT_EQUAL(0, UiGauge::remainingPercent(140.0f));
}

/** Verifies gauge colors describe remaining capacity, not consumed usage. */
void testUsageGaugeColorUsesRemainingCapacity(void) {
    TEST_ASSERT_EQUAL(UiGauge::UsageGaugeColor::Green, UiGauge::colorForUsedPercent(0.0f));
    TEST_ASSERT_EQUAL(UiGauge::UsageGaugeColor::Green, UiGauge::colorForUsedPercent(49.0f));
    TEST_ASSERT_EQUAL(UiGauge::UsageGaugeColor::Amber, UiGauge::colorForUsedPercent(50.0f));
    TEST_ASSERT_EQUAL(UiGauge::UsageGaugeColor::Red, UiGauge::colorForUsedPercent(80.0f));
    TEST_ASSERT_EQUAL(UiGauge::UsageGaugeColor::Red, UiGauge::colorForUsedPercent(100.0f));
}

/** Runs the usage gauge native test suite. */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(testUsageGaugeShowsRemainingPercent);
    RUN_TEST(testUsageGaugeClampsRemainingPercent);
    RUN_TEST(testUsageGaugeColorUsesRemainingCapacity);
    return UNITY_END();
}
