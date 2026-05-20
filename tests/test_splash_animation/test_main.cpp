#include <unity.h>

#include "splash_animation.h"

/** Verifies horizontal distortion bands stay inside display-safe dimensions. */
void testDistortionBandsStayDisplaySafe(void) {
    for (uint16_t frame = 0; frame < kSplashDistortionFrameCount * 2U; ++frame) {
        for (uint8_t band = 0; band < kSplashDistortionBandCount; ++band) {
            TEST_ASSERT_LESS_THAN(kSplashScreenHeight, splashDistortionBandY(frame, band));
            TEST_ASSERT_GREATER_OR_EQUAL(kSplashDistortionBandMinHeight,
                                         splashDistortionBandHeight(frame, band));
            TEST_ASSERT_LESS_OR_EQUAL(kSplashDistortionBandMaxHeight,
                                      splashDistortionBandHeight(frame, band));
            TEST_ASSERT_GREATER_OR_EQUAL(kSplashDistortionBandMinWidth,
                                         splashDistortionBandWidth(frame, band));
            TEST_ASSERT_LESS_OR_EQUAL(kSplashDistortionBandMaxWidth,
                                      splashDistortionBandWidth(frame, band));
            TEST_ASSERT_GREATER_OR_EQUAL(-kSplashDistortionBandMaxOffsetX,
                                         splashDistortionBandOffsetX(frame, band));
            TEST_ASSERT_LESS_OR_EQUAL(kSplashDistortionBandMaxOffsetX,
                                      splashDistortionBandOffsetX(frame, band));
        }
    }
}

/** Verifies distortion bands pulse between quiet and bright frames. */
void testDistortionBandsPulseLikeInterference(void) {
    bool sawQuietBand = false;
    bool sawBrightBand = false;

    for (uint16_t frame = 0; frame < kSplashDistortionFrameCount * 2U; ++frame) {
        for (uint8_t band = 0; band < kSplashDistortionBandCount; ++band) {
            uint8_t opacity = splashDistortionBandOpacity(frame, band);
            TEST_ASSERT_LESS_OR_EQUAL(kSplashDistortionBandMaxOpacity, opacity);
            sawQuietBand = sawQuietBand || opacity <= kSplashDistortionBandQuietOpacity;
            sawBrightBand = sawBrightBand || opacity >= kSplashDistortionBandBurstOpacity;
        }
    }

    TEST_ASSERT_TRUE(sawQuietBand);
    TEST_ASSERT_TRUE(sawBrightBand);
}

/** Verifies distortion strength stays below the comfort ceiling. */
void testDistortionStrengthStaysModerate(void) {
    TEST_ASSERT_LESS_OR_EQUAL(14, kSplashDistortionBandCount);
    TEST_ASSERT_LESS_OR_EQUAL(6, kSplashDistortionBandMaxHeight);
    TEST_ASSERT_LESS_OR_EQUAL(32, kSplashDistortionBandMaxOffsetX);
    TEST_ASSERT_LESS_OR_EQUAL(160, kSplashDistortionBandMaxOpacity);
    TEST_ASSERT_LESS_OR_EQUAL(112, kSplashDistortionBandBurstOpacity);
    TEST_ASSERT_LESS_OR_EQUAL(30, kSplashTextSliceMaxOffsetX);
    TEST_ASSERT_LESS_OR_EQUAL(184, kSplashTextSliceMaxOpacity);
    TEST_ASSERT_LESS_OR_EQUAL(5, kSplashTextJitterMaxOffsetX);
}

/** Verifies the privacy animation uses a calmer frame cadence. */
void testSplashFrameCadenceIsCalmer(void) {
    TEST_ASSERT_GREATER_OR_EQUAL(115, kSplashFrameIntervalMs);
    TEST_ASSERT_LESS_OR_EQUAL(140, kSplashFrameIntervalMs);
}

/** Verifies the animation does not visibly loop in the first few seconds. */
void testDistortionDoesNotRepeatAfterShortWindow(void) {
    uint8_t repeatedFrames = 0;

    TEST_ASSERT_GREATER_THAN(180, kSplashDistortionFrameCount);
    for (uint16_t frame = 0; frame < 64; ++frame) {
        bool sameAsShortLoop = splashDistortionBandY(frame, 3) == splashDistortionBandY(frame + 48, 3) &&
                               splashDistortionBandOffsetX(frame, 7) ==
                                   splashDistortionBandOffsetX(frame + 48, 7) &&
                               splashTextSliceOffsetX(frame, 1, 2) ==
                                   splashTextSliceOffsetX(frame + 48, 1, 2);
        if (sameAsShortLoop) repeatedFrames++;
    }

    TEST_ASSERT_LESS_THAN(4, repeatedFrames);
}

/** Verifies every screensaver text row gets visible sliced distortion. */
void testAllTextRowsUseVisibleSliceDistortion(void) {
    for (uint8_t label = 0; label < kSplashDistortedTextLabelCount; ++label) {
        bool sawLargeNegativeOffset = false;
        bool sawLargePositiveOffset = false;

        for (uint16_t frame = 0; frame < kSplashDistortionFrameCount; ++frame) {
            for (uint8_t slice = 0; slice < kSplashTextSliceCount; ++slice) {
                int8_t offset = splashTextSliceOffsetX(frame, label, slice);
                TEST_ASSERT_GREATER_OR_EQUAL(-kSplashTextSliceMaxOffsetX, offset);
                TEST_ASSERT_LESS_OR_EQUAL(kSplashTextSliceMaxOffsetX, offset);
                sawLargeNegativeOffset = sawLargeNegativeOffset || offset <= -12;
                sawLargePositiveOffset = sawLargePositiveOffset || offset >= 12;
            }
        }

        TEST_ASSERT_TRUE(sawLargeNegativeOffset);
        TEST_ASSERT_TRUE(sawLargePositiveOffset);
    }
}

/** Verifies each text slice remains within its clipping area. */
void testTextSlicesStayInsideLabelArea(void) {
    int8_t labelHeights[kSplashDistortedTextLabelCount] = {22, 48, 18, 18, 18, 18};

    for (uint16_t frame = 0; frame < kSplashDistortionFrameCount; ++frame) {
        for (uint8_t label = 0; label < kSplashDistortedTextLabelCount; ++label) {
            for (uint8_t slice = 0; slice < kSplashTextSliceCount; ++slice) {
                int8_t y = splashTextSliceY(frame, label, slice, labelHeights[label]);
                int8_t height = splashTextSliceHeight(frame, label, slice, labelHeights[label]);
                TEST_ASSERT_LESS_THAN(labelHeights[label], y);
                TEST_ASSERT_GREATER_OR_EQUAL(kSplashTextSliceMinHeight, height);
                TEST_ASSERT_LESS_OR_EQUAL(kSplashTextSliceMaxHeight, height);
                TEST_ASSERT_LESS_OR_EQUAL(labelHeights[label], y + height);
            }
        }
    }
}

/** Verifies supporting labels jitter subtly without leaving their row. */
void testSupportingTextJitterIsReadable(void) {
    bool sawJitter = false;

    for (uint16_t frame = 0; frame < kSplashDistortionFrameCount; ++frame) {
        for (uint8_t label = 0; label < kSplashDistortedTextLabelCount; ++label) {
            int8_t offset = splashTextJitterOffsetX(frame, label);
            TEST_ASSERT_GREATER_OR_EQUAL(-kSplashTextJitterMaxOffsetX, offset);
            TEST_ASSERT_LESS_OR_EQUAL(kSplashTextJitterMaxOffsetX, offset);
            sawJitter = sawJitter || offset != 0;
        }
    }

    TEST_ASSERT_TRUE(sawJitter);
}

/** Runs the splash distortion native test suite. */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(testDistortionBandsStayDisplaySafe);
    RUN_TEST(testDistortionBandsPulseLikeInterference);
    RUN_TEST(testDistortionStrengthStaysModerate);
    RUN_TEST(testSplashFrameCadenceIsCalmer);
    RUN_TEST(testDistortionDoesNotRepeatAfterShortWindow);
    RUN_TEST(testAllTextRowsUseVisibleSliceDistortion);
    RUN_TEST(testTextSlicesStayInsideLabelArea);
    RUN_TEST(testSupportingTextJitterIsReadable);
    return UNITY_END();
}
