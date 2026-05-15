#pragma once

#include <stdint.h>

/**
 * Starts M5Unified with the serial baud rate used by the firmware.
 */
void hardwareInit(uint32_t serialBaud);

/**
 * Pumps M5Unified input and power state updates.
 */
void hardwareUpdate();

/**
 * Samples the IMU and applies landscape rotation changes.
 */
bool hardwareAutoRotate();

/**
 * Returns the active display width in pixels.
 */
uint16_t displayWidth();

/**
 * Returns the active display height in pixels.
 */
uint16_t displayHeight();

/**
 * Sets the CoreS3 display backlight brightness.
 */
void setDisplayBrightness(uint8_t brightness);

/**
 * Clears the display to black before LVGL takes over drawing.
 */
void fillDisplayBlack();

/**
 * Flushes an RGB565 LVGL draw buffer rectangle to the M5 display.
 */
void flushDisplayRgb565(int32_t x, int32_t y, int32_t width, int32_t height, const uint16_t *pixels);

/**
 * Reads the current touch point when the display is pressed.
 */
bool readTouchPoint(int16_t *x, int16_t *y);

/**
 * Returns battery percentage, or -1 when the value is unavailable.
 */
int getBatteryPercent();

/**
 * Returns true while USB or external power is charging the battery.
 */
bool isBatteryCharging();

/**
 * Returns true when the CoreS3 power button was clicked.
 */
bool wasPowerClicked();
