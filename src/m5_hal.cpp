#include "m5_hal.h"

#include <Arduino.h>
#include <M5Unified.h>

#include "orientation.h"

static constexpr uint32_t kAutoRotateIntervalMs = 250;
static DisplayRotation currentRotation = DisplayRotationLandscape;

/** Starts M5Unified and applies the default display rotation. */
void hardwareInit(uint32_t serialBaud) {
    auto config = M5.config();
    config.serial_baudrate = serialBaud;
    config.clear_display = true;
    config.output_power = true;
    M5.begin(config);
    M5.Display.setRotation(currentRotation);
}

/** Updates M5Unified button, touch, power, and sensor state. */
void hardwareUpdate() {
    M5.update();
}

/** Samples the IMU at a fixed interval and rotates the display when needed. */
bool hardwareAutoRotate() {
    static uint32_t lastSampleMs = 0;
    uint32_t now = millis();
    if (now - lastSampleMs < kAutoRotateIntervalMs) return false;
    lastSampleMs = now;

    if (!M5.Imu.isEnabled()) return false;

    float ax = 0.0f;
    float ay = 0.0f;
    float az = 0.0f;
    if (!M5.Imu.getAccel(&ax, &ay, &az)) return false;

    DisplayRotation nextRotation = orientationFromAccel(ax, ay, az, currentRotation);
    if (nextRotation == currentRotation) return false;

    // Clear stale LVGL pixels after the panel rotation changes.
    currentRotation = nextRotation;
    M5.Display.setRotation(currentRotation);
    M5.Display.fillScreen(TFT_BLACK);
    return true;
}

/** Returns the active M5 display width in pixels. */
uint16_t displayWidth() {
    return static_cast<uint16_t>(M5.Display.width());
}

/** Returns the active M5 display height in pixels. */
uint16_t displayHeight() {
    return static_cast<uint16_t>(M5.Display.height());
}

/** Sets the M5 display brightness level. */
void setDisplayBrightness(uint8_t brightness) {
    M5.Display.setBrightness(brightness);
}

/** Clears the M5 display to black. */
void fillDisplayBlack() {
    M5.Display.fillScreen(TFT_BLACK);
}

/** Writes an LVGL RGB565 rectangle to the M5 display. */
void flushDisplayRgb565(int32_t x, int32_t y, int32_t width, int32_t height, const uint16_t *pixels) {
    M5.Display.startWrite();
    M5.Display.setAddrWindow(x, y, width, height);
    M5.Display.writePixels(reinterpret_cast<const lgfx::rgb565_t *>(pixels),
                           static_cast<uint32_t>(width * height));
    M5.Display.endWrite();
}

/** Reads the current M5 touch point when pressed. */
bool readTouchPoint(int16_t *x, int16_t *y) {
    auto touch = M5.Touch.getDetail();
    if (!touch.isPressed()) return false;
    if (x) *x = touch.x;
    if (y) *y = touch.y;
    return true;
}

/** Returns normalized battery percentage, or -1 for invalid readings. */
int getBatteryPercent() {
    int percent = M5.Power.getBatteryLevel();
    if (percent < 0 || percent > 100) return -1;
    return percent;
}

/** Returns whether the M5 power subsystem reports charging. */
bool isBatteryCharging() {
    return M5.Power.isCharging() == m5::Power_Class::is_charging;
}

/** Returns whether the M5 power subsystem reports an attached battery. */
bool hasBatteryAttachment() {
#if defined(CONFIG_IDF_TARGET_ESP32S3)
    if (M5.Power.getType() == m5::Power_Class::pmic_axp2101) {
        return M5.Power.Axp2101.getBatState();
    }
#endif
    return getBatteryPercent() >= 0;
}

/** Returns whether the CoreS3 power button was clicked. */
bool wasPowerClicked() {
    return M5.BtnPWR.wasClicked();
}
