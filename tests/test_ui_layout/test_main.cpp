#include <unity.h>

#include "ui_layout.h"

/** Verifies the usage panels leave room for the footer area. */
void testUsagePanelsFitWithoutClippingFooter(void) {
    TEST_ASSERT_LESS_OR_EQUAL(UiLayout::kFooterTop,
                              UiLayout::kContentY + (UiLayout::kPanelHeight * 2) + UiLayout::kPanelGap);
}

/** Verifies reset labels stay inside their usage panel. */
void testResetLabelStaysInsidePanelContent(void) {
    TEST_ASSERT_EQUAL(50, UiLayout::kResetTextY);
    TEST_ASSERT_LESS_OR_EQUAL(UiLayout::kPanelHeight - UiLayout::kPanelInnerBottom,
                              UiLayout::kResetTextY + UiLayout::kResetTextHeight);
}

/** Verifies usage panel contents use the requested raised alignment. */
void testUsagePanelContentsUseRaisedAlignment(void) {
    TEST_ASSERT_EQUAL(2, UiLayout::kPanelPercentTextY);
    TEST_ASSERT_EQUAL(8, UiLayout::kPanelPillTextY);
    TEST_ASSERT_EQUAL(36, UiLayout::kPanelBarY);
    TEST_ASSERT_LESS_THAN(UiLayout::kResetTextY, UiLayout::kPanelBarY);
}

/** Verifies no layered glow bands are enabled for the background. */
void testBackgroundUsesNoLayeredGlowBands(void) {
    TEST_ASSERT_EQUAL(0, UiLayout::kBackdropGlowBands);
}

/** Verifies gradient backgrounds remain disabled for the display. */
void testBackgroundGradientIsDisabledForLimitedColorDisplay(void) {
    TEST_ASSERT_FALSE(UiLayout::kBackgroundGradientEnabled);
}

/** Verifies usage panels keep the intended taller height. */
void testUsagePanelsAreTallerForMoreBreathingRoom(void) {
    TEST_ASSERT_TRUE(UiLayout::kPanelHeight >= 72);
}

/** Verifies bottom status text remains compact. */
void testBottomStatusTextUsesSmallerFont(void) {
    TEST_ASSERT_LESS_OR_EQUAL(14, UiLayout::kStatusTextFontPx);
}

/** Verifies the dot loader and footer text fit in the footer row. */
void testFooterLoaderAndTextFitFooterWidth(void) {
    TEST_ASSERT_LESS_OR_EQUAL(UiLayout::kPanelWidth,
                              UiLayout::kFooterLoaderWidth + UiLayout::kFooterLoaderTextGap +
                                  UiLayout::kFooterTextMaxWidth);
}

/** Verifies footer loader dots are large enough to read as dots on-device. */
void testFooterLoaderDotsAreReadableOnDevice(void) {
    TEST_ASSERT_GREATER_OR_EQUAL(6, UiLayout::kFooterLoaderDotSize);
}

/** Verifies footer loader dots avoid glow artifacts on the physical display. */
void testFooterLoaderAvoidsBloomGlow(void) {
    TEST_ASSERT_EQUAL(0, UiLayout::kFooterLoaderDotShadowWidth);
}

/** Verifies footer loader dots use the requested lower visual alignment. */
void testFooterLoaderDotsMoveDownTwoPixels(void) {
    int dotY = ((UiLayout::kFooterLoaderHeight - UiLayout::kFooterLoaderDotSize) / 2) +
               UiLayout::kFooterLoaderDotOffsetY;

    TEST_ASSERT_EQUAL(2, UiLayout::kFooterLoaderDotOffsetY);
    TEST_ASSERT_GREATER_OR_EQUAL(0, dotY);
    TEST_ASSERT_LESS_OR_EQUAL(UiLayout::kFooterLoaderHeight, dotY + UiLayout::kFooterLoaderDotSize);
}

/** Verifies the shared brand mark fits before the header accent line. */
void testBrandMarkFitsHeaderAccent(void) {
    TEST_ASSERT_EQUAL(-1, UiLayout::kBrandMarkOffsetY);
    TEST_ASSERT_EQUAL(0, UiLayout::kBrandMarkBorderWidth);
    TEST_ASSERT_LESS_OR_EQUAL(43,
                              UiLayout::kTitleY + UiLayout::kBrandMarkOffsetY + UiLayout::kBrandMarkHeight);
    TEST_ASSERT_LESS_OR_EQUAL(UiLayout::kScreenWidth / 4, UiLayout::kBrandMarkWidth);
}

/** Verifies the shared brand mark keeps the updated host gauge proportions. */
void testBrandMarkGaugeFitsInsideContainer(void) {
    TEST_ASSERT_LESS_OR_EQUAL(UiLayout::kBrandMarkWidth,
                              UiLayout::kBrandMarkGaugeX + UiLayout::kBrandMarkGaugeSize);
    TEST_ASSERT_LESS_OR_EQUAL(UiLayout::kBrandMarkHeight,
                              UiLayout::kBrandMarkGaugeY + (UiLayout::kBrandMarkGaugeSize / 2) +
                                  UiLayout::kBrandMarkArcWidth);
    TEST_ASSERT_GREATER_THAN(UiLayout::kBrandMarkAccentArcWidth, UiLayout::kBrandMarkArcWidth);
    TEST_ASSERT_GREATER_THAN(UiLayout::kBrandMarkAccentArcEndAngle,
                             UiLayout::kBrandMarkArcEndAngle);
}

/** Verifies the shared brand mark needle points from hub to orange node. */
void testBrandMarkNeedleTracksHostIconDiagonal(void) {
    TEST_ASSERT_GREATER_THAN(UiLayout::kBrandMarkNodeY, UiLayout::kBrandMarkHubY);
    TEST_ASSERT_GREATER_THAN(UiLayout::kBrandMarkHubX, UiLayout::kBrandMarkNodeX);
    TEST_ASSERT_GREATER_THAN(UiLayout::kBrandMarkGaugeSize / 2, UiLayout::kBrandMarkNeedleHeight);
    TEST_ASSERT_EQUAL(380, UiLayout::kBrandMarkNeedleRotation);
}

/** Verifies the shared brand mark baseline and endpoint dots remain readable. */
void testBrandMarkBaseAndDotsFitUpdatedIcon(void) {
    TEST_ASSERT_LESS_OR_EQUAL(UiLayout::kBrandMarkWidth,
                              UiLayout::kBrandMarkBaseX + UiLayout::kBrandMarkBaseWidth);
    TEST_ASSERT_LESS_OR_EQUAL(UiLayout::kBrandMarkHeight,
                              UiLayout::kBrandMarkBaseY + UiLayout::kBrandMarkBaseHeight);
    TEST_ASSERT_LESS_THAN(UiLayout::kBrandMarkRightDotX, UiLayout::kBrandMarkLeftDotX);
    TEST_ASSERT_LESS_THAN(UiLayout::kBrandMarkBaseY, UiLayout::kBrandMarkDotY);
}

/** Verifies the waiting overlay keeps shared header status visible. */
void testSharedHeaderStaysVisibleWhileWaitingForConnection(void) {
    TEST_ASSERT_TRUE(UiLayout::sharedHeaderIsVisible(false, false));
    TEST_ASSERT_TRUE(UiLayout::sharedHeaderIsVisible(false, true));
    TEST_ASSERT_FALSE(UiLayout::sharedHeaderIsVisible(true, false));
    TEST_ASSERT_FALSE(UiLayout::sharedHeaderIsVisible(true, true));
}

/** Runs the UI layout native test suite. */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(testUsagePanelsFitWithoutClippingFooter);
    RUN_TEST(testResetLabelStaysInsidePanelContent);
    RUN_TEST(testUsagePanelContentsUseRaisedAlignment);
    RUN_TEST(testBackgroundUsesNoLayeredGlowBands);
    RUN_TEST(testBackgroundGradientIsDisabledForLimitedColorDisplay);
    RUN_TEST(testUsagePanelsAreTallerForMoreBreathingRoom);
    RUN_TEST(testBottomStatusTextUsesSmallerFont);
    RUN_TEST(testFooterLoaderAndTextFitFooterWidth);
    RUN_TEST(testFooterLoaderDotsAreReadableOnDevice);
    RUN_TEST(testFooterLoaderAvoidsBloomGlow);
    RUN_TEST(testFooterLoaderDotsMoveDownTwoPixels);
    RUN_TEST(testBrandMarkFitsHeaderAccent);
    RUN_TEST(testBrandMarkGaugeFitsInsideContainer);
    RUN_TEST(testBrandMarkNeedleTracksHostIconDiagonal);
    RUN_TEST(testBrandMarkBaseAndDotsFitUpdatedIcon);
    RUN_TEST(testSharedHeaderStaysVisibleWhileWaitingForConnection);
    return UNITY_END();
}
