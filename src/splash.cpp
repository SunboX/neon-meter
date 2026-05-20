#include "splash.h"

#include "firmware_info.h"
#include "splash_animation.h"
#include "theme.h"

#include <Arduino.h>
#include <stdio.h>

static constexpr int kTitleLabelX = 10;
static constexpr int kTitleLabelY = 72;
static constexpr int kTitleLabelWidth = 300;
static constexpr int kModeLabelX = 0;
static constexpr int kModeLabelY = 40;
static constexpr int kMetadataLabelX = 26;
static constexpr int kFirmwareLabelY = 154;
static constexpr int kHardwareLabelY = 174;
static constexpr int kConnectionLabelY = 194;
static constexpr int kHiddenLabelX = 0;
static constexpr int kHiddenLabelY = 218;
static constexpr int8_t kModeLabelHeight = 22;
static constexpr int8_t kTitleLabelHeight = 48;
static constexpr int8_t kMetadataLabelHeight = 18;
static constexpr int8_t kHiddenLabelHeight = 18;

enum SplashTextLabelIndex {
    SplashTextPrivacy = 0,
    SplashTextTitle,
    SplashTextFirmware,
    SplashTextHardware,
    SplashTextConnection,
    SplashTextHidden
};

static lv_obj_t *splashRoot = nullptr;
static lv_obj_t *distortionBands[kSplashDistortionBandCount] = {};
static lv_obj_t *textLabels[kSplashDistortedTextLabelCount] = {};
static lv_obj_t *textSliceClips[kSplashDistortedTextLabelCount][kSplashTextSliceCount] = {};
static lv_obj_t *textSliceLabels[kSplashDistortedTextLabelCount][kSplashTextSliceCount] = {};
static bool splashActive = false;
static bool splashUsbConnected = false;
static BleState splashBleState = BleStateInit;
static uint16_t frameIndex = 0;
static uint32_t lastFrameMs = 0;

/** Applies the same flat background used by the main UI. */
static void applySplashBackdrop(lv_obj_t *object) {
    lv_obj_set_style_bg_color(object, kThemeBg, 0);
    lv_obj_set_style_bg_grad_dir(object, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_opa(object, LV_OPA_COVER, 0);
}

/** Creates one transparent clipping container for a distorted title slice. */
static lv_obj_t *makeSliceClip(lv_obj_t *parent) {
    lv_obj_t *clip = lv_obj_create(parent);
    lv_obj_remove_style_all(clip);
    lv_obj_set_style_bg_opa(clip, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(clip, 0, 0);
    lv_obj_set_style_pad_all(clip, 0, 0);
    lv_obj_clear_flag(clip, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(clip, LV_OBJ_FLAG_EVENT_BUBBLE);
    return clip;
}

/** Returns the left edge for one screensaver text row. */
static int textLabelX(uint8_t labelIndex) {
    switch (labelIndex) {
    case SplashTextTitle:
        return kTitleLabelX;
    case SplashTextFirmware:
    case SplashTextHardware:
    case SplashTextConnection:
        return kMetadataLabelX;
    case SplashTextHidden:
        return kHiddenLabelX;
    default:
        return kModeLabelX;
    }
}

/** Returns the top edge for one screensaver text row. */
static int textLabelY(uint8_t labelIndex) {
    switch (labelIndex) {
    case SplashTextTitle:
        return kTitleLabelY;
    case SplashTextFirmware:
        return kFirmwareLabelY;
    case SplashTextHardware:
        return kHardwareLabelY;
    case SplashTextConnection:
        return kConnectionLabelY;
    case SplashTextHidden:
        return kHiddenLabelY;
    default:
        return kModeLabelY;
    }
}

/** Returns the width for one screensaver text row. */
static int textLabelWidth(uint8_t labelIndex) {
    switch (labelIndex) {
    case SplashTextTitle:
        return kTitleLabelWidth;
    case SplashTextFirmware:
        return 180;
    case SplashTextHardware:
        return 220;
    case SplashTextConnection:
        return 238;
    default:
        return kSplashScreenWidth;
    }
}

/** Returns the clipping height used for one screensaver text row. */
static int8_t textLabelHeight(uint8_t labelIndex) {
    switch (labelIndex) {
    case SplashTextTitle:
        return kTitleLabelHeight;
    case SplashTextFirmware:
    case SplashTextHardware:
    case SplashTextConnection:
        return kMetadataLabelHeight;
    case SplashTextHidden:
        return kHiddenLabelHeight;
    default:
        return kModeLabelHeight;
    }
}

/** Creates one cyberpunk screensaver text label. */
static lv_obj_t *makeSplashLabel(lv_obj_t *parent, const char *text, const lv_font_t *font,
                                 lv_color_t color, int x, int y, int width,
                                 lv_text_align_t align) {
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_width(label, width);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(label, align, 0);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, color, 0);
    lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(label, 0, 0);
    lv_obj_set_pos(label, x, y);
    lv_obj_add_flag(label, LV_OBJ_FLAG_EVENT_BUBBLE);
    return label;
}

/** Updates one base text label and its sliced distortion copies. */
static void setDistortedText(uint8_t labelIndex, const char *text) {
    if (labelIndex >= kSplashDistortedTextLabelCount) return;
    if (textLabels[labelIndex]) lv_label_set_text(textLabels[labelIndex], text);
    for (uint8_t slice = 0; slice < kSplashTextSliceCount; ++slice) {
        if (textSliceLabels[labelIndex][slice]) {
            lv_label_set_text(textSliceLabels[labelIndex][slice], text);
        }
    }
}

/** Returns the alternating color used by one horizontal interference band. */
static lv_color_t distortionBandColor(uint8_t bandIndex) {
    switch (bandIndex % 4U) {
    case 0:
        return kThemeAccent;
    case 1:
        return kThemeBlue;
    case 2:
        return kThemeGreen;
    default:
        return kThemeOrange;
    }
}

/** Paints moving horizontal interference bands across the privacy overlay. */
static void renderDistortionBands() {
    for (uint8_t band = 0; band < kSplashDistortionBandCount; ++band) {
        if (!distortionBands[band]) continue;
        lv_obj_set_size(distortionBands[band],
                        splashDistortionBandWidth(frameIndex, band),
                        splashDistortionBandHeight(frameIndex, band));
        lv_obj_set_pos(distortionBands[band],
                       splashDistortionBandOffsetX(frameIndex, band),
                       splashDistortionBandY(frameIndex, band));
        lv_obj_set_style_bg_opa(distortionBands[band],
                                static_cast<lv_opa_t>(splashDistortionBandOpacity(frameIndex, band)), 0);
    }
}

/** Paints jitter on the readable base text labels. */
static void renderTextJitter() {
    for (uint8_t label = 0; label < kSplashDistortedTextLabelCount; ++label) {
        if (!textLabels[label]) continue;
        lv_obj_set_pos(textLabels[label],
                       textLabelX(label) + splashTextJitterOffsetX(frameIndex, label),
                       textLabelY(label));
    }
}

/** Paints clipped duplicate rows for all screensaver text labels. */
static void renderTextSlices() {
    for (uint8_t label = 0; label < kSplashDistortedTextLabelCount; ++label) {
        int8_t areaHeight = textLabelHeight(label);
        int clipX = textLabelX(label) - kSplashTextSliceMaxOffsetX;
        int clipWidth = textLabelWidth(label) + (2 * kSplashTextSliceMaxOffsetX);
        for (uint8_t slice = 0; slice < kSplashTextSliceCount; ++slice) {
            if (!textSliceClips[label][slice] || !textSliceLabels[label][slice]) continue;
            int sliceY = splashTextSliceY(frameIndex, label, slice, areaHeight);
            int sliceHeight = splashTextSliceHeight(frameIndex, label, slice, areaHeight);
            int labelX = kSplashTextSliceMaxOffsetX + splashTextSliceOffsetX(frameIndex, label, slice);
            int labelY = -sliceY + splashTextSliceOffsetY(frameIndex, label, slice);

            lv_obj_set_size(textSliceClips[label][slice], clipWidth, sliceHeight);
            lv_obj_set_pos(textSliceClips[label][slice], clipX, textLabelY(label) + sliceY);
            lv_obj_set_pos(textSliceLabels[label][slice], labelX, labelY);
            lv_obj_set_style_text_opa(textSliceLabels[label][slice],
                                      static_cast<lv_opa_t>(splashTextSliceOpacity(frameIndex, label, slice)), 0);
        }
    }
}

/** Paints the current privacy distortion frame. */
static void renderFrame() {
    if (!splashRoot) return;
    renderDistortionBands();
    renderTextJitter();
    renderTextSlices();
}

/** Builds the splash LVGL object tree and hides it by default. */
void splashInit(lv_obj_t *parent) {
    splashRoot = lv_obj_create(parent);
    lv_obj_set_size(splashRoot, kSplashScreenWidth, kSplashScreenHeight);
    lv_obj_set_pos(splashRoot, 0, 0);
    applySplashBackdrop(splashRoot);
    lv_obj_set_style_border_width(splashRoot, 0, 0);
    lv_obj_set_style_pad_all(splashRoot, 0, 0);
    lv_obj_clear_flag(splashRoot, LV_OBJ_FLAG_SCROLLABLE);

    for (uint8_t band = 0; band < kSplashDistortionBandCount; ++band) {
        distortionBands[band] = lv_obj_create(splashRoot);
        lv_obj_remove_style_all(distortionBands[band]);
        lv_obj_set_style_bg_color(distortionBands[band], distortionBandColor(band), 0);
        lv_obj_set_style_bg_opa(distortionBands[band], LV_OPA_30, 0);
        lv_obj_set_style_radius(distortionBands[band], 0, 0);
        lv_obj_set_style_border_width(distortionBands[band], 0, 0);
        lv_obj_set_style_shadow_width(distortionBands[band], band % 5 == 0 ? 12 : 5, 0);
        lv_obj_set_style_shadow_color(distortionBands[band], distortionBandColor(band), 0);
        lv_obj_set_style_shadow_opa(distortionBands[band], LV_OPA_30, 0);
        lv_obj_set_style_pad_all(distortionBands[band], 0, 0);
        lv_obj_clear_flag(distortionBands[band], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(distortionBands[band], LV_OBJ_FLAG_EVENT_BUBBLE);
    }

    textLabels[SplashTextPrivacy] = makeSplashLabel(splashRoot, kScreensaverPrivacyText,
                                                    &lv_font_montserrat_16, kThemeOrange,
                                                    kModeLabelX, kModeLabelY, kSplashScreenWidth,
                                                    LV_TEXT_ALIGN_CENTER);
    textLabels[SplashTextTitle] = makeSplashLabel(splashRoot, kScreensaverTitleText,
                                                  &lv_font_montserrat_32, kThemeAccent,
                                                  kTitleLabelX, kTitleLabelY, kTitleLabelWidth,
                                                  LV_TEXT_ALIGN_CENTER);
    lv_obj_set_style_shadow_width(textLabels[SplashTextTitle], 12, 0);
    lv_obj_set_style_shadow_color(textLabels[SplashTextTitle], kThemeAccent, 0);
    lv_obj_set_style_shadow_opa(textLabels[SplashTextTitle], LV_OPA_50, 0);

    char firmwareText[32];
    snprintf(firmwareText, sizeof(firmwareText), "FW v%s", kFirmwareVersion);
    textLabels[SplashTextFirmware] = makeSplashLabel(splashRoot, firmwareText, &lv_font_montserrat_14,
                                                     kThemeDim, kMetadataLabelX, kFirmwareLabelY, 180,
                                                     LV_TEXT_ALIGN_LEFT);
    textLabels[SplashTextHardware] = makeSplashLabel(splashRoot, kScreensaverHardwareText,
                                                     &lv_font_montserrat_14, kThemeDim,
                                                     kMetadataLabelX, kHardwareLabelY, 220,
                                                     LV_TEXT_ALIGN_LEFT);
    textLabels[SplashTextConnection] = makeSplashLabel(
        splashRoot, screensaverConnectionText(splashBleState, splashUsbConnected),
        &lv_font_montserrat_14, kThemeGreen, kMetadataLabelX, kConnectionLabelY, 238,
        LV_TEXT_ALIGN_LEFT);
    textLabels[SplashTextHidden] = makeSplashLabel(splashRoot, kScreensaverHiddenText,
                                                   &lv_font_montserrat_14, kThemeOrange,
                                                   kHiddenLabelX, kHiddenLabelY, kSplashScreenWidth,
                                                   LV_TEXT_ALIGN_CENTER);

    for (uint8_t label = 0; label < kSplashDistortedTextLabelCount; ++label) {
        for (uint8_t slice = 0; slice < kSplashTextSliceCount; ++slice) {
            lv_color_t color = slice % 3 == 0 ? kThemeBlue : (slice % 3 == 1 ? kThemeOrange : kThemeAccent);
            textSliceClips[label][slice] = makeSliceClip(splashRoot);
            textSliceLabels[label][slice] = makeSplashLabel(textSliceClips[label][slice], "",
                                                            label == SplashTextTitle ? &lv_font_montserrat_32
                                                                                     : &lv_font_montserrat_14,
                                                            color, 0, 0, textLabelWidth(label),
                                                            label == SplashTextFirmware ||
                                                                    label == SplashTextHardware ||
                                                                    label == SplashTextConnection
                                                                ? LV_TEXT_ALIGN_LEFT
                                                                : LV_TEXT_ALIGN_CENTER);
            lv_obj_set_style_text_opa(textSliceLabels[label][slice], LV_OPA_TRANSP, 0);
        }
    }

    setDistortedText(SplashTextPrivacy, kScreensaverPrivacyText);
    setDistortedText(SplashTextTitle, kScreensaverTitleText);
    setDistortedText(SplashTextFirmware, firmwareText);
    setDistortedText(SplashTextHardware, kScreensaverHardwareText);
    setDistortedText(SplashTextConnection, screensaverConnectionText(splashBleState, splashUsbConnected));
    setDistortedText(SplashTextHidden, kScreensaverHiddenText);

    renderFrame();
    lv_obj_add_flag(splashRoot, LV_OBJ_FLAG_HIDDEN);
}

/** Shows the splash root and enables animation ticks. */
void splashShow() {
    if (!splashRoot) return;
    splashActive = true;
    lv_obj_clear_flag(splashRoot, LV_OBJ_FLAG_HIDDEN);
}

/** Hides the splash root and disables animation ticks. */
void splashHide() {
    if (!splashRoot) return;
    splashActive = false;
    lv_obj_add_flag(splashRoot, LV_OBJ_FLAG_HIDDEN);
}

/** Updates the privacy-safe connection status text. */
void splashSetStatus(BleState state, bool usbConnected) {
    splashBleState = state;
    splashUsbConnected = usbConnected;
    setDistortedText(SplashTextConnection, screensaverConnectionText(state, usbConnected));
}

/** Advances the splash frame after the animation interval elapses. */
void splashTick() {
    if (!splashActive) return;
    uint32_t now = millis();
    if (now - lastFrameMs < kSplashFrameIntervalMs) return;
    lastFrameMs = now;
    frameIndex = static_cast<uint8_t>((frameIndex + 1) % kSplashDistortionFrameCount);
    renderFrame();
}

/** Advances the splash frame immediately for button-driven cycling. */
void splashNext() {
    frameIndex = static_cast<uint8_t>((frameIndex + 1) % kSplashDistortionFrameCount);
    lastFrameMs = millis();
    renderFrame();
}

/** Returns whether the splash screen is active. */
bool isSplashActive() {
    return splashActive;
}

/** Returns the splash root object for event hookup. */
lv_obj_t *getSplashRoot() {
    return splashRoot;
}
