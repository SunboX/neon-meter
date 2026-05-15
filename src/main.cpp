#include <Arduino.h>
#include <lvgl.h>
#include <esp_heap_caps.h>

#include "ble_service.h"
#include "m5_hal.h"
#include "splash.h"
#include "ui.h"
#include "usage_model.h"
#include "usage_rate.h"

static constexpr uint32_t kSerialBaud = 115200;
static constexpr uint16_t kBufferLines = 40;
static lv_color_t *drawBuffer1 = nullptr;
static lv_color_t *drawBuffer2 = nullptr;
static UsageData usage = {};
static UsageBundle usageBundle = {};
static UsageRateTracker rateTracker;
static size_t activeUsageIndex = 0;
static uint32_t lastUsageRotationMs = 0;

/** Gives LVGL the Arduino millis() clock. */
static uint32_t lvTick() {
    return millis();
}

/** Flushes LVGL's RGB565 pixel map to the M5 display. */
static void lvFlush(lv_display_t *displayObject, const lv_area_t *area, uint8_t *pixelMap) {
    int32_t width = area->x2 - area->x1 + 1;
    int32_t height = area->y2 - area->y1 + 1;
    flushDisplayRgb565(area->x1, area->y1, width, height, reinterpret_cast<const uint16_t *>(pixelMap));
    lv_display_flush_ready(displayObject);
}

/** Reads touch state for LVGL's pointer input driver. */
static void lvTouchRead(lv_indev_t *inputDevice, lv_indev_data_t *data) {
    (void)inputDevice;
    int16_t x = 0;
    int16_t y = 0;
    if (readTouchPoint(&x, &y)) {
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

/** Allocates LVGL draw buffers, preferring PSRAM when it is available. */
static void *allocDrawBuffer(size_t bytes) {
    void *buffer = heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!buffer) buffer = heap_caps_malloc(bytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    return buffer;
}

/** Starts LVGL display and touch drivers. */
static void initLvgl() {
    lv_init();
    lv_tick_set_cb(lvTick);

    uint32_t width = displayWidth();
    uint32_t height = displayHeight();
    size_t bufferSize = width * kBufferLines * sizeof(lv_color_t);
    drawBuffer1 = static_cast<lv_color_t *>(allocDrawBuffer(bufferSize));
    drawBuffer2 = static_cast<lv_color_t *>(allocDrawBuffer(bufferSize));
    if (!drawBuffer1 || !drawBuffer2) {
        Serial.println("LVGL draw buffer allocation failed");
        while (true) delay(1000);
    }

    lv_display_t *display = lv_display_create(width, height);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(display, lvFlush);
    lv_display_set_buffers(display, drawBuffer1, drawBuffer2, bufferSize, LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_indev_t *touch = lv_indev_create();
    lv_indev_set_type(touch, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touch, lvTouchRead);
}

/** Applies one cached provider payload to the UI and rate tracker. */
static void applyUsageItem(size_t index, bool trackRate) {
    if (index >= usageBundle.count) return;
    usage = usageBundle.items[index];
    if (trackRate) {
        rateTracker.addSample(millis(), usage.primaryPct);
    }
    uiUpdate(&usage, rateTracker.getGroup());
    if (!trackRate) return;
    Serial.printf("Payload accepted: provider=%s primary=%.1f secondary=%.1f group=%d\n",
                  usage.provider, usage.primaryPct, usage.secondaryPct, rateTracker.getGroup());
}

/** Parses and applies one provider bundle from BLE or serial. */
static void handlePayload(const char *json) {
    UsageBundle next = {};
    if (!parseUsageBundleJson(json, &next)) {
        Serial.println("Payload parse failed");
        sendBleNack();
        return;
    }

    usageBundle = next;
    activeUsageIndex = 0;
    lastUsageRotationMs = millis();
    applyUsageItem(activeUsageIndex, true);
    sendBleAck();
    Serial.printf("Provider bundle accepted: count=%u rotationMs=%lu\n",
                  static_cast<unsigned>(usageBundle.count),
                  static_cast<unsigned long>(usageBundle.rotationMs));
}

/** Rotates between cached provider payloads when the host sent more than one. */
static void rotateUsageBundleIfNeeded() {
    if (usageBundle.count < 2) return;
    uint32_t now = millis();
    if (now - lastUsageRotationMs < usageBundle.rotationMs) return;
    lastUsageRotationMs = now;
    activeUsageIndex = (activeUsageIndex + 1) % usageBundle.count;
    applyUsageItem(activeUsageIndex, false);
    Serial.printf("Provider screen rotated: provider=%s index=%u\n",
                  usage.provider, static_cast<unsigned>(activeUsageIndex));
}

/** Reads newline-delimited JSON payloads from USB serial. */
static void handleSerialJson() {
    static char serialBuffer[640];
    static size_t serialPos = 0;

    while (Serial.available()) {
        char incoming = static_cast<char>(Serial.read());
        if (incoming == '\n' || incoming == '\r') {
            if (serialPos > 0) {
                serialBuffer[serialPos] = '\0';
                if (serialBuffer[0] == '{') handlePayload(serialBuffer);
                serialPos = 0;
            }
        } else if (serialPos < sizeof(serialBuffer) - 1) {
            serialBuffer[serialPos++] = incoming;
        }
    }
}

/** Arduino setup hook that initializes hardware, BLE, LVGL, and the first screen. */
void setup() {
    hardwareInit(kSerialBaud);
    Serial.begin(kSerialBaud);
    delay(250);
    Serial.println("{\"ready\":true,\"device\":\"Neon Meter\"}");

    setDisplayBrightness(180);
    fillDisplayBlack();

    initLvgl();
    bleInit();
    uiInit();
    uiUpdateBleStatus(getBleState(), getBleDeviceName(), getBleAddress());
    uiUpdateBattery(getBatteryPercent(), isBatteryCharging());
    uiShowScreen(ScreenUsage);

    Serial.println("Neon Meter ready");
}

/** Arduino loop hook that pumps hardware, UI, BLE, serial, and button events. */
void loop() {
    hardwareUpdate();
    if (hardwareAutoRotate()) {
        lv_obj_invalidate(lv_screen_active());
    }
    lv_timer_handler();
    uiTickAnimation();
    splashTick();
    rotateUsageBundleIfNeeded();
    bleTick();
    handleSerialJson();

    static BleState lastBleState = BleStateInit;
    BleState bleState = getBleState();
    if (bleState != lastBleState) {
        lastBleState = bleState;
        uiUpdateBleStatus(bleState, getBleDeviceName(), getBleAddress());
    }

    static uint32_t lastBatteryMs = 0;
    if (millis() - lastBatteryMs >= 2000) {
        lastBatteryMs = millis();
        uiUpdateBattery(getBatteryPercent(), isBatteryCharging());
    }

    if (wasPowerClicked()) {
        if (uiCurrentScreen() == ScreenSplash) splashNext();
        else uiCycleScreen();
    }

    if (bleHasData()) {
        handlePayload(readBleData());
    }

    delay(5);
}
