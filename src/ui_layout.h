#pragma once

namespace UiLayout {
constexpr int kScreenWidth = 320;
constexpr int kScreenHeight = 240;
constexpr int kMargin = 10;
constexpr int kTitleY = 10;
constexpr int kContentY = 56;
constexpr int kPanelWidth = kScreenWidth - 2 * kMargin;
constexpr int kPanelHeight = 72;
constexpr int kPanelGap = 8;
constexpr int kFooterTop = 208;
constexpr int kPanelInnerBottom = 2;
constexpr int kPanelPercentTextY = 2;
constexpr int kPanelPillTextY = 8;
constexpr int kPanelBarX = 24;
constexpr int kPanelBarY = 36;
constexpr int kPanelBarWidth = kPanelWidth - 44;
constexpr int kInfoTextOffset = 8;
constexpr int kInfoStatusTextX = kInfoTextOffset;
constexpr int kInfoStatusTextY = 6;
constexpr int kInfoDeviceTextY = 42;
constexpr int kInfoAddressTextY = 62;
constexpr int kResetTextY = 50;
constexpr int kResetTextHeight = 14;
constexpr int kStatusTextFontPx = 14;
constexpr int kFooterHeight = 18;
constexpr int kFooterLoaderDotSize = 6;
constexpr int kFooterLoaderDotGap = 3;
constexpr int kFooterLoaderWidth = (kFooterLoaderDotSize * 3) + (kFooterLoaderDotGap * 2);
constexpr int kFooterLoaderHeight = 12;
constexpr int kFooterLoaderDotOffsetY = 2;
constexpr int kFooterLoaderDotShadowWidth = 0;
constexpr int kFooterLoaderTextGap = 8;
constexpr int kFooterTextMaxWidth = 250;
constexpr int kBrandMarkOffsetY = -1;
constexpr int kBrandMarkWidth = 46;
constexpr int kBrandMarkHeight = 30;
constexpr int kBrandMarkBorderWidth = 0;
constexpr int kBrandMarkGaugeSize = 34;
constexpr int kBrandMarkGaugeX = 6;
constexpr int kBrandMarkGaugeY = 6;
constexpr int kBrandMarkArcWidth = 4;
constexpr int kBrandMarkAccentArcWidth = 2;
constexpr int kBrandMarkArcStartAngle = 180;
constexpr int kBrandMarkArcEndAngle = 360;
constexpr int kBrandMarkAccentArcEndAngle = 318;
constexpr int kBrandMarkNeedleWidth = 4;
constexpr int kBrandMarkNeedleHeight = 24;
constexpr int kBrandMarkNeedleX = 21;
constexpr int kBrandMarkNeedleY = 3;
constexpr int kBrandMarkNeedleRotation = 380;
constexpr int kBrandMarkHubSize = 8;
constexpr int kBrandMarkHubX = 13;
constexpr int kBrandMarkHubY = 20;
constexpr int kBrandMarkHubCoreSize = 4;
constexpr int kBrandMarkNodeSize = 5;
constexpr int kBrandMarkNodeX = 30;
constexpr int kBrandMarkNodeY = 4;
constexpr int kBrandMarkDotSize = 3;
constexpr int kBrandMarkLeftDotX = 9;
constexpr int kBrandMarkRightDotX = 34;
constexpr int kBrandMarkDotY = 22;
constexpr int kBrandMarkBaseWidth = 24;
constexpr int kBrandMarkBaseHeight = 2;
constexpr int kBrandMarkBaseX = 12;
constexpr int kBrandMarkBaseY = 25;
constexpr int kBackdropGlowBands = 0;
constexpr bool kBackgroundGradientEnabled = false;

/** Returns the Weekly panel Y position for one- or two-window layouts. */
constexpr int secondaryPanelY(bool sessionEnabled) {
    return sessionEnabled ? kContentY + kPanelHeight + kPanelGap : kContentY;
}

/** Returns whether shared header status belongs above the current overlay. */
constexpr bool sharedHeaderIsVisible(bool splashVisible, bool /*waitingForConnection*/) {
    return !splashVisible;
}

/** Returns whether the battery label belongs in the shared header. */
constexpr bool batteryHeaderIsVisible(bool sharedHeaderVisible, bool batteryAttached) {
    return sharedHeaderVisible && batteryAttached;
}
}
