#include "splash_animation.h"

/** Returns the shortest circular distance between the active sweep and a bar. */
static uint8_t splashSweepDistance(uint8_t frame, uint8_t barIndex) {
    uint8_t active = static_cast<uint8_t>(frame % kSplashSweepBarCount);
    uint8_t bar = static_cast<uint8_t>(barIndex % kSplashSweepBarCount);
    uint8_t forward = active > bar ? static_cast<uint8_t>(active - bar) : static_cast<uint8_t>(bar - active);
    uint8_t wrapped = static_cast<uint8_t>(kSplashSweepBarCount - forward);
    return forward < wrapped ? forward : wrapped;
}

/** Returns the opacity for one bar in the splash neon sweep animation. */
uint8_t splashSweepOpacity(uint8_t frame, uint8_t barIndex) {
    switch (splashSweepDistance(frame, barIndex)) {
    case 0:
        return kSplashSweepPeakOpacity;
    case 1:
        return kSplashSweepShoulderOpacity;
    case 2:
        return kSplashSweepTrailOpacity;
    case 3:
        return kSplashSweepHaloOpacity;
    default:
        return kSplashSweepBaseOpacity;
    }
}

/** Returns the height for one bar in the splash neon sweep animation. */
uint8_t splashSweepHeight(uint8_t frame, uint8_t barIndex) {
    switch (splashSweepDistance(frame, barIndex)) {
    case 0:
        return kSplashSweepPeakHeight;
    case 1:
        return kSplashSweepShoulderHeight;
    case 2:
        return kSplashSweepTrailHeight;
    case 3:
        return kSplashSweepHaloHeight;
    default:
        return kSplashSweepBaseHeight;
    }
}
