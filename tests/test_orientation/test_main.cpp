#include <unity.h>

#include "orientation.h"

/** Verifies positive landscape gravity keeps the default rotation. */
void testPositiveLandscapeAxisUsesDefaultLandscapeRotation(void) {
    TEST_ASSERT_EQUAL(DisplayRotationLandscape,
                      orientationFromLandscapeAxis(0.8f, DisplayRotationLandscapeFlipped));
}

/** Verifies negative landscape gravity selects the flipped rotation. */
void testNegativeLandscapeAxisUsesFlippedLandscapeRotation(void) {
    TEST_ASSERT_EQUAL(DisplayRotationLandscapeFlipped,
                      orientationFromLandscapeAxis(-0.8f, DisplayRotationLandscape));
}

/** Verifies neutral gravity leaves the current rotation unchanged. */
void testNeutralLandscapeAxisKeepsCurrentRotation(void) {
    TEST_ASSERT_EQUAL(DisplayRotationLandscapeFlipped,
                      orientationFromLandscapeAxis(0.1f, DisplayRotationLandscapeFlipped));
}

/** Verifies accelerometer mapping uses the Y axis for landscape flips. */
void testAccelerometerUsesYAxisForLandscapeFlip(void) {
    TEST_ASSERT_EQUAL(DisplayRotationLandscapeFlipped,
                      orientationFromAccel(0.0f, -0.9f, 0.1f, DisplayRotationLandscape));
}

/** Runs the orientation native test suite. */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(testPositiveLandscapeAxisUsesDefaultLandscapeRotation);
    RUN_TEST(testNegativeLandscapeAxisUsesFlippedLandscapeRotation);
    RUN_TEST(testNeutralLandscapeAxisKeepsCurrentRotation);
    RUN_TEST(testAccelerometerUsesYAxisForLandscapeFlip);
    return UNITY_END();
}
