#pragma once

#include "ble_service.h"

#include <lvgl.h>

/**
 * Creates the splash screen objects under the active LVGL screen.
 */
void splashInit(lv_obj_t *parent);

/**
 * Shows the splash screen and enables its animation.
 */
void splashShow();

/**
 * Hides the splash screen and stops its animation.
 */
void splashHide();

/**
 * Advances the splash animation when its frame interval has elapsed.
 */
void splashTick();

/**
 * Updates the privacy-safe connection metadata shown on the splash screen.
 */
void splashSetStatus(BleState state, bool usbConnected);

/**
 * Forces the splash animation to the next frame.
 */
void splashNext();

/**
 * Returns true when the splash screen is currently visible.
 */
bool isSplashActive();

/**
 * Returns the splash root object so callers can attach LVGL events.
 */
lv_obj_t *getSplashRoot();
