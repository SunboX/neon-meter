#include "orientation.h"

/** Chooses standard or flipped landscape when gravity exceeds the threshold. */
DisplayRotation orientationFromLandscapeAxis(float gravityAxis, DisplayRotation current) {
    if (gravityAxis >= kOrientationRotationThresholdG) return DisplayRotationLandscape;
    if (gravityAxis <= -kOrientationRotationThresholdG) return DisplayRotationLandscapeFlipped;
    return current;
}

/** Uses the CoreS3 Y-axis acceleration as the landscape flip input. */
DisplayRotation orientationFromAccel(float ax, float ay, float az, DisplayRotation current) {
    (void)ax;
    (void)az;
    // CoreS3 landscape flip is driven by the Y axis when the device is upright.
    return orientationFromLandscapeAxis(ay, current);
}
