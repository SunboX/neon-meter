#include <unity.h>

#include "ui_footer_loader.h"

/** Verifies non-blank frames light exactly one dot. */
void testFooterLoaderLightsOnlyOneDotPerFrame(void) {
    for (uint8_t frame = 0; frame < kFooterLoaderDotCount; ++frame) {
        uint8_t activeCount = 0;
        for (uint8_t dot = 0; dot < kFooterLoaderDotCount; ++dot) {
            if (footerLoaderDotOpacity(frame, dot) == kFooterLoaderActiveOpacity) {
                activeCount++;
            } else {
                TEST_ASSERT_EQUAL(kFooterLoaderInactiveOpacity, footerLoaderDotOpacity(frame, dot));
            }
        }
        TEST_ASSERT_EQUAL(1, activeCount);
    }
}

/** Verifies the wrap frame goes dark before the first dot lights again. */
void testFooterLoaderHasBlankWrapFrame(void) {
    uint8_t blankFrame = kFooterLoaderFrameCount - 1;

    TEST_ASSERT_GREATER_THAN(kFooterLoaderDotCount, kFooterLoaderFrameCount);
    for (uint8_t dot = 0; dot < kFooterLoaderDotCount; ++dot) {
        TEST_ASSERT_EQUAL(kFooterLoaderInactiveOpacity, footerLoaderDotOpacity(blankFrame, dot));
    }
}

/** Verifies inactive dots remain visible but do not look active. */
void testFooterLoaderInactiveDotsStayVisible(void) {
    TEST_ASSERT_GREATER_THAN(0, kFooterLoaderInactiveOpacity);
    TEST_ASSERT_LESS_THAN(kFooterLoaderActiveOpacity, kFooterLoaderInactiveOpacity);
}

/** Runs the footer loader native test suite. */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(testFooterLoaderLightsOnlyOneDotPerFrame);
    RUN_TEST(testFooterLoaderHasBlankWrapFrame);
    RUN_TEST(testFooterLoaderInactiveDotsStayVisible);
    return UNITY_END();
}
