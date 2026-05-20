#include "splash_animation.h"

/** Returns a compact deterministic hash value for animation variation. */
static uint8_t splashNoise(uint16_t frame, uint16_t salt) {
    uint32_t value = 0x9e3779b9UL;
    value ^= static_cast<uint32_t>(frame % kSplashDistortionFrameCount) * 0x85ebca6bUL;
    value ^= static_cast<uint32_t>(salt) * 0xc2b2ae35UL;
    value ^= value >> 16;
    value *= 0x7feb352dUL;
    value ^= value >> 15;
    value *= 0x846ca68bUL;
    value ^= value >> 16;
    return static_cast<uint8_t>(value & 0xffU);
}

/** Returns a signed value inside an inclusive symmetric range. */
static int16_t signedNoise(uint16_t frame, uint16_t salt, int16_t magnitude) {
    int16_t span = static_cast<int16_t>((magnitude * 2) + 1);
    return static_cast<int16_t>((splashNoise(frame, salt) % span) - magnitude);
}

/** Returns true when a distortion element is in its bright pulse phase. */
static bool distortionBurstActive(uint16_t frame, uint8_t index) {
    uint8_t noise = splashNoise(frame, static_cast<uint16_t>(index + 277U));
    return noise < 42 || noise > 244;
}

/** Returns the y position for one horizontal distortion band. */
int16_t splashDistortionBandY(uint16_t frame, uint8_t bandIndex) {
    uint16_t base = static_cast<uint16_t>(bandIndex) * 19U;
    uint16_t drift = static_cast<uint16_t>(frame % kSplashDistortionFrameCount) *
                     static_cast<uint16_t>(1U + (bandIndex % 4U));
    uint16_t jitter = splashNoise(frame, static_cast<uint16_t>(bandIndex + 11U)) % 9U;
    return static_cast<int16_t>((base + drift + jitter) % kSplashScreenHeight);
}

/** Returns the height for one horizontal distortion band. */
int8_t splashDistortionBandHeight(uint16_t frame, uint8_t bandIndex) {
    uint8_t span = static_cast<uint8_t>(kSplashDistortionBandMaxHeight -
                                        kSplashDistortionBandMinHeight + 1);
    uint8_t value = splashNoise(frame, static_cast<uint16_t>(bandIndex + 29U)) % span;
    return static_cast<int8_t>(kSplashDistortionBandMinHeight + value);
}

/** Returns the width for one horizontal distortion band. */
int16_t splashDistortionBandWidth(uint16_t frame, uint8_t bandIndex) {
    uint16_t span = static_cast<uint16_t>(kSplashDistortionBandMaxWidth -
                                          kSplashDistortionBandMinWidth + 1);
    uint16_t value = splashNoise(frame, static_cast<uint16_t>(bandIndex + 43U)) % span;
    return static_cast<int16_t>(kSplashDistortionBandMinWidth + value);
}

/** Returns the horizontal offset for one distortion band. */
int16_t splashDistortionBandOffsetX(uint16_t frame, uint8_t bandIndex) {
    return signedNoise(frame, static_cast<uint16_t>(bandIndex + 61U),
                       kSplashDistortionBandMaxOffsetX);
}

/** Returns the opacity for one horizontal distortion band. */
uint8_t splashDistortionBandOpacity(uint16_t frame, uint8_t bandIndex) {
    uint8_t noise = splashNoise(frame, static_cast<uint16_t>(bandIndex + 83U));
    if (distortionBurstActive(frame, bandIndex)) {
        return static_cast<uint8_t>(kSplashDistortionBandBurstOpacity +
                                    (noise % (kSplashDistortionBandMaxOpacity -
                                              kSplashDistortionBandBurstOpacity + 1U)));
    }
    if (splashNoise(frame, static_cast<uint16_t>(bandIndex + 313U)) < 52) {
        return static_cast<uint8_t>(34U + (noise % 34U));
    }
    return static_cast<uint8_t>(8U + (noise % (kSplashDistortionBandQuietOpacity - 7U)));
}

/** Returns true while a text slice should visibly tear away from the base text. */
static bool textSliceActive(uint16_t frame, uint8_t labelIndex, uint8_t sliceIndex) {
    uint8_t noise = splashNoise(frame, static_cast<uint16_t>(401U + (labelIndex * 37U) + sliceIndex));
    return noise < 112 || noise > 238;
}

/** Returns the clipped height for one text label slice. */
int8_t splashTextSliceHeight(uint16_t frame, uint8_t labelIndex, uint8_t sliceIndex, int8_t areaHeight) {
    int8_t maxHeight = areaHeight < kSplashTextSliceMaxHeight ? areaHeight : kSplashTextSliceMaxHeight;
    int8_t minHeight = areaHeight < kSplashTextSliceMinHeight ? areaHeight : kSplashTextSliceMinHeight;
    uint8_t span = static_cast<uint8_t>(maxHeight - minHeight + 1);
    uint8_t value = splashNoise(frame, static_cast<uint16_t>(503U + (labelIndex * 41U) + sliceIndex)) % span;
    return static_cast<int8_t>(minHeight + value);
}

/** Returns the y offset inside a text label area for one clipped slice. */
int8_t splashTextSliceY(uint16_t frame, uint8_t labelIndex, uint8_t sliceIndex, int8_t areaHeight) {
    int8_t height = splashTextSliceHeight(frame, labelIndex, sliceIndex, areaHeight);
    uint8_t span = static_cast<uint8_t>(areaHeight - height + 1);
    uint8_t value = splashNoise(frame, static_cast<uint16_t>(607U + (labelIndex * 43U) + sliceIndex)) % span;
    return static_cast<int8_t>(value);
}

/** Returns the horizontal text offset for one clipped text label slice. */
int8_t splashTextSliceOffsetX(uint16_t frame, uint8_t labelIndex, uint8_t sliceIndex) {
    if (!textSliceActive(frame, labelIndex, sliceIndex)) return 0;
    return static_cast<int8_t>(signedNoise(frame, static_cast<uint16_t>(701U + (labelIndex * 47U) + sliceIndex),
                                           kSplashTextSliceMaxOffsetX));
}

/** Returns the vertical text offset for one clipped text label slice. */
int8_t splashTextSliceOffsetY(uint16_t frame, uint8_t labelIndex, uint8_t sliceIndex) {
    if (!textSliceActive(frame, labelIndex, sliceIndex)) return 0;
    return static_cast<int8_t>(signedNoise(frame, static_cast<uint16_t>(809U + (labelIndex * 53U) + sliceIndex), 2));
}

/** Returns the opacity for one clipped text label slice. */
uint8_t splashTextSliceOpacity(uint16_t frame, uint8_t labelIndex, uint8_t sliceIndex) {
    if (!textSliceActive(frame, labelIndex, sliceIndex)) return 0;
    uint8_t noise = splashNoise(frame, static_cast<uint16_t>(919U + (labelIndex * 59U) + sliceIndex));
    return static_cast<uint8_t>(84U + (noise % (kSplashTextSliceMaxOpacity - 83U)));
}

/** Returns the subtle horizontal jitter for supporting text labels. */
int8_t splashTextJitterOffsetX(uint16_t frame, uint8_t labelIndex) {
    uint8_t noise = splashNoise(frame, static_cast<uint16_t>(1013U + (labelIndex * 61U)));
    if (noise > 58 && noise < 226) return 0;
    return static_cast<int8_t>(signedNoise(frame, static_cast<uint16_t>(1103U + (labelIndex * 67U)),
                                           kSplashTextJitterMaxOffsetX));
}
