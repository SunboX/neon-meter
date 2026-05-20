#pragma once

#include <stdint.h>

static constexpr int16_t kSplashScreenWidth = 320;
static constexpr int16_t kSplashScreenHeight = 240;
static constexpr uint16_t kSplashDistortionFrameCount = 251;
static constexpr uint32_t kSplashFrameIntervalMs = 125;
static constexpr uint8_t kSplashDistortionBandCount = 14;
static constexpr int16_t kSplashDistortionBandMinWidth = 120;
static constexpr int16_t kSplashDistortionBandMaxWidth = kSplashScreenWidth;
static constexpr int8_t kSplashDistortionBandMinHeight = 1;
static constexpr int8_t kSplashDistortionBandMaxHeight = 6;
static constexpr int16_t kSplashDistortionBandMaxOffsetX = 32;
static constexpr uint8_t kSplashDistortionBandQuietOpacity = 22;
static constexpr uint8_t kSplashDistortionBandBurstOpacity = 104;
static constexpr uint8_t kSplashDistortionBandMaxOpacity = 152;
static constexpr uint8_t kSplashDistortedTextLabelCount = 6;
static constexpr uint8_t kSplashTextSliceCount = 5;
static constexpr int8_t kSplashTextSliceMinHeight = 3;
static constexpr int8_t kSplashTextSliceMaxHeight = 12;
static constexpr int8_t kSplashTextSliceMaxOffsetX = 28;
static constexpr uint8_t kSplashTextSliceMaxOpacity = 176;
static constexpr int8_t kSplashTextJitterMaxOffsetX = 5;

/**
 * Returns the y position for one horizontal distortion band.
 */
int16_t splashDistortionBandY(uint16_t frame, uint8_t bandIndex);

/**
 * Returns the height for one horizontal distortion band.
 */
int8_t splashDistortionBandHeight(uint16_t frame, uint8_t bandIndex);

/**
 * Returns the width for one horizontal distortion band.
 */
int16_t splashDistortionBandWidth(uint16_t frame, uint8_t bandIndex);

/**
 * Returns the horizontal offset for one distortion band.
 */
int16_t splashDistortionBandOffsetX(uint16_t frame, uint8_t bandIndex);

/**
 * Returns the opacity for one horizontal distortion band.
 */
uint8_t splashDistortionBandOpacity(uint16_t frame, uint8_t bandIndex);

/**
 * Returns the y offset inside a text label area for one clipped slice.
 */
int8_t splashTextSliceY(uint16_t frame, uint8_t labelIndex, uint8_t sliceIndex, int8_t areaHeight);

/**
 * Returns the clipped height for one text label slice.
 */
int8_t splashTextSliceHeight(uint16_t frame, uint8_t labelIndex, uint8_t sliceIndex, int8_t areaHeight);

/**
 * Returns the horizontal text offset for one clipped text label slice.
 */
int8_t splashTextSliceOffsetX(uint16_t frame, uint8_t labelIndex, uint8_t sliceIndex);

/**
 * Returns the vertical text offset for one clipped text label slice.
 */
int8_t splashTextSliceOffsetY(uint16_t frame, uint8_t labelIndex, uint8_t sliceIndex);

/**
 * Returns the opacity for one clipped text label slice.
 */
uint8_t splashTextSliceOpacity(uint16_t frame, uint8_t labelIndex, uint8_t sliceIndex);

/**
 * Returns the subtle horizontal jitter for supporting text labels.
 */
int8_t splashTextJitterOffsetX(uint16_t frame, uint8_t labelIndex);
