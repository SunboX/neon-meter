#pragma once

#include <stdint.h>

static constexpr uint8_t kSplashSweepBarCount = 18;
static constexpr uint8_t kSplashSweepFrameCount = kSplashSweepBarCount;
static constexpr uint8_t kSplashSweepPeakOpacity = 240;
static constexpr uint8_t kSplashSweepShoulderOpacity = 190;
static constexpr uint8_t kSplashSweepTrailOpacity = 130;
static constexpr uint8_t kSplashSweepHaloOpacity = 76;
static constexpr uint8_t kSplashSweepBaseOpacity = 38;
static constexpr uint8_t kSplashSweepPeakHeight = 76;
static constexpr uint8_t kSplashSweepShoulderHeight = 52;
static constexpr uint8_t kSplashSweepTrailHeight = 34;
static constexpr uint8_t kSplashSweepHaloHeight = 24;
static constexpr uint8_t kSplashSweepBaseHeight = 18;

/**
 * Returns the opacity for one bar in the splash neon sweep animation.
 */
uint8_t splashSweepOpacity(uint8_t frame, uint8_t barIndex);

/**
 * Returns the height for one bar in the splash neon sweep animation.
 */
uint8_t splashSweepHeight(uint8_t frame, uint8_t barIndex);
