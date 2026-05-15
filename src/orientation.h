#pragma once

#include <stdint.h>

/**
 * Landscape display rotations supported by the CoreS3 firmware.
 */
enum DisplayRotation : uint8_t {
    DisplayRotationLandscape = 1,
    DisplayRotationLandscapeFlipped = 3,
};

static constexpr float kOrientationRotationThresholdG = 0.55f;

/**
 * Chooses the landscape display rotation from one gravity axis.
 */
DisplayRotation orientationFromLandscapeAxis(float gravityAxis, DisplayRotation current);

/**
 * Chooses the landscape display rotation from accelerometer readings.
 */
DisplayRotation orientationFromAccel(float ax, float ay, float az, DisplayRotation current);
