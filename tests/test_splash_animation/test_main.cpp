#include <unity.h>

#include "splash_animation.h"

/** Verifies every animation frame has exactly one brightest sweep bar. */
void testSplashSweepHasSinglePeakPerFrame(void) {
    for (uint8_t frame = 0; frame < kSplashSweepFrameCount; ++frame) {
        uint8_t peakCount = 0;
        for (uint8_t bar = 0; bar < kSplashSweepBarCount; ++bar) {
            if (splashSweepOpacity(frame, bar) == kSplashSweepPeakOpacity) {
                peakCount++;
                TEST_ASSERT_EQUAL(kSplashSweepPeakHeight, splashSweepHeight(frame, bar));
            }
        }
        TEST_ASSERT_EQUAL(1, peakCount);
    }
}

/** Verifies the sweep fades out from the active bar instead of blinking. */
void testSplashSweepFadesFromPeak(void) {
    TEST_ASSERT_GREATER_THAN(splashSweepOpacity(0, 1), splashSweepOpacity(0, 0));
    TEST_ASSERT_GREATER_THAN(splashSweepOpacity(0, 2), splashSweepOpacity(0, 1));
    TEST_ASSERT_GREATER_THAN(splashSweepOpacity(0, 4), splashSweepOpacity(0, 2));

    TEST_ASSERT_GREATER_THAN(splashSweepHeight(0, 1), splashSweepHeight(0, 0));
    TEST_ASSERT_GREATER_THAN(splashSweepHeight(0, 2), splashSweepHeight(0, 1));
    TEST_ASSERT_GREATER_THAN(splashSweepHeight(0, 4), splashSweepHeight(0, 2));
}

/** Verifies the neon sweep wraps smoothly across the first and last bars. */
void testSplashSweepWrapsAcrossEdges(void) {
    TEST_ASSERT_EQUAL(splashSweepOpacity(0, 1), splashSweepOpacity(0, kSplashSweepBarCount - 1));
    TEST_ASSERT_EQUAL(splashSweepHeight(0, 1), splashSweepHeight(0, kSplashSweepBarCount - 1));
}

/** Verifies frame indexes normalize so long-running animation cannot drift. */
void testSplashSweepNormalizesFrameIndex(void) {
    for (uint8_t bar = 0; bar < kSplashSweepBarCount; ++bar) {
        TEST_ASSERT_EQUAL(splashSweepOpacity(0, bar), splashSweepOpacity(kSplashSweepFrameCount, bar));
        TEST_ASSERT_EQUAL(splashSweepHeight(0, bar), splashSweepHeight(kSplashSweepFrameCount, bar));
    }
}

/** Runs the splash animation native test suite. */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(testSplashSweepHasSinglePeakPerFrame);
    RUN_TEST(testSplashSweepFadesFromPeak);
    RUN_TEST(testSplashSweepWrapsAcrossEdges);
    RUN_TEST(testSplashSweepNormalizesFrameIndex);
    return UNITY_END();
}
