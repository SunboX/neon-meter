#pragma once

#include "ble_service.h"
#include "usage_model.h"

#include <lvgl.h>

/**
 * Top-level screens available in the firmware UI.
 */
enum Screen {
    ScreenSplash = 0,
    ScreenUsage,
    ScreenBluetooth
};

/**
 * Creates all LVGL screen objects and shared header widgets.
 */
void uiInit();

/**
 * Updates usage labels and progress bars from parsed provider data.
 */
void uiUpdate(const UsageData *data, int rateGroup);

/**
 * Advances the footer animation while the usage screen is visible.
 */
void uiTickAnimation();

/**
 * Updates Bluetooth status text, device name, and address.
 */
void uiUpdateBleStatus(BleState state, const char *name, const char *address);

/**
 * Updates whether USB serial has an active protocol host.
 */
void uiUpdateUsbStatus(bool connected);

/**
 * Updates the battery indicator text.
 */
void uiUpdateBattery(int percent, bool charging);

/**
 * Shows one top-level firmware screen.
 */
void uiShowScreen(Screen screen);

/**
 * Cycles between usage and Bluetooth screens.
 */
void uiCycleScreen();

/**
 * Toggles the splash screen over the previous non-splash screen.
 */
void uiToggleSplash();

/**
 * Returns the currently visible top-level screen.
 */
Screen uiCurrentScreen();
