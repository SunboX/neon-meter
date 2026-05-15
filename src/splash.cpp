#include "splash.h"

#include "splash_animation.h"
#include "theme.h"

#include <Arduino.h>

static constexpr int kScreenWidth = 320;
static constexpr int kScreenHeight = 240;
static constexpr int kSweepBarWidth = 10;
static constexpr int kSweepBarGap = 4;
static constexpr int kSweepTotalWidth =
    (kSplashSweepBarCount * kSweepBarWidth) + ((kSplashSweepBarCount - 1) * kSweepBarGap);
static constexpr int kSweepStartX = (kScreenWidth - kSweepTotalWidth) / 2;
static constexpr int kSweepCenterY = 108;
static constexpr int kSweepTopRailY = 58;
static constexpr int kSweepBottomRailY = 158;
static constexpr uint32_t kSplashFrameIntervalMs = 95;

static lv_obj_t *splashRoot = nullptr;
static lv_obj_t *sweepBars[kSplashSweepBarCount] = {};
static lv_obj_t *caption = nullptr;
static bool splashActive = false;
static uint8_t frameIndex = 0;
static uint32_t lastFrameMs = 0;

/** Applies the same flat background used by the main UI. */
static void applySplashBackdrop(lv_obj_t *object) {
    lv_obj_set_style_bg_color(object, kThemeBg, 0);
    lv_obj_set_style_bg_grad_dir(object, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_opa(object, LV_OPA_COVER, 0);
}

/** Returns the neon color for one sweep opacity band. */
static lv_color_t sweepColor(uint8_t opacity) {
    if (opacity >= kSplashSweepShoulderOpacity) return kThemeAccent;
    if (opacity >= kSplashSweepTrailOpacity) return kThemeBlue;
    if (opacity >= kSplashSweepHaloOpacity) return kThemeOrange;
    return kThemePanel2;
}

/** Returns the glow strength for one sweep opacity band. */
static lv_opa_t sweepShadowOpacity(uint8_t opacity) {
    if (opacity >= kSplashSweepShoulderOpacity) return LV_OPA_70;
    if (opacity >= kSplashSweepTrailOpacity) return LV_OPA_40;
    if (opacity >= kSplashSweepHaloOpacity) return LV_OPA_20;
    return LV_OPA_TRANSP;
}

/** Creates one low-profile meter rail behind the sweep bars. */
static void makeMeterRail(lv_obj_t *parent, int y, lv_color_t color) {
    lv_obj_t *rail = lv_obj_create(parent);
    lv_obj_set_size(rail, kSweepTotalWidth + 16, 2);
    lv_obj_set_pos(rail, kSweepStartX - 8, y);
    lv_obj_set_style_bg_color(rail, color, 0);
    lv_obj_set_style_bg_opa(rail, LV_OPA_40, 0);
    lv_obj_set_style_radius(rail, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(rail, 0, 0);
    lv_obj_set_style_shadow_width(rail, 10, 0);
    lv_obj_set_style_shadow_color(rail, color, 0);
    lv_obj_set_style_shadow_opa(rail, LV_OPA_20, 0);
    lv_obj_set_style_pad_all(rail, 0, 0);
    lv_obj_clear_flag(rail, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(rail, LV_OBJ_FLAG_EVENT_BUBBLE);
}

/** Paints the current neon sweep frame into the reusable LVGL bar objects. */
static void renderFrame() {
    if (!splashRoot) return;
    for (uint8_t bar = 0; bar < kSplashSweepBarCount; ++bar) {
        if (!sweepBars[bar]) continue;
        uint8_t opacity = splashSweepOpacity(frameIndex, bar);
        uint8_t height = splashSweepHeight(frameIndex, bar);
        lv_color_t color = sweepColor(opacity);
        int x = kSweepStartX + (bar * (kSweepBarWidth + kSweepBarGap));
        int y = kSweepCenterY - (height / 2);

        lv_obj_set_size(sweepBars[bar], kSweepBarWidth, height);
        lv_obj_set_pos(sweepBars[bar], x, y);
        lv_obj_set_style_bg_color(sweepBars[bar], color, 0);
        lv_obj_set_style_bg_opa(sweepBars[bar], static_cast<lv_opa_t>(opacity), 0);
        lv_obj_set_style_shadow_color(sweepBars[bar], color, 0);
        lv_obj_set_style_shadow_opa(sweepBars[bar], sweepShadowOpacity(opacity), 0);
    }
}

/** Builds the splash LVGL object tree and hides it by default. */
void splashInit(lv_obj_t *parent) {
    splashRoot = lv_obj_create(parent);
    lv_obj_set_size(splashRoot, kScreenWidth, kScreenHeight);
    lv_obj_set_pos(splashRoot, 0, 0);
    applySplashBackdrop(splashRoot);
    lv_obj_set_style_border_width(splashRoot, 0, 0);
    lv_obj_set_style_pad_all(splashRoot, 0, 0);
    lv_obj_clear_flag(splashRoot, LV_OBJ_FLAG_SCROLLABLE);

    makeMeterRail(splashRoot, kSweepTopRailY, kThemeAccent);
    makeMeterRail(splashRoot, kSweepBottomRailY, kThemeOrange);

    for (uint8_t bar = 0; bar < kSplashSweepBarCount; ++bar) {
        sweepBars[bar] = lv_obj_create(splashRoot);
        lv_obj_set_size(sweepBars[bar], kSweepBarWidth, kSplashSweepBaseHeight);
        lv_obj_set_pos(sweepBars[bar], kSweepStartX + bar * (kSweepBarWidth + kSweepBarGap),
                       kSweepCenterY - (kSplashSweepBaseHeight / 2));
        lv_obj_set_style_radius(sweepBars[bar], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(sweepBars[bar], 0, 0);
        lv_obj_set_style_shadow_width(sweepBars[bar], 14, 0);
        lv_obj_set_style_pad_all(sweepBars[bar], 0, 0);
        lv_obj_clear_flag(sweepBars[bar], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(sweepBars[bar], LV_OBJ_FLAG_EVENT_BUBBLE);
    }

    caption = lv_label_create(splashRoot);
    lv_label_set_text(caption, "Neon Meter");
    lv_obj_set_style_text_font(caption, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(caption, kThemeAccent, 0);
    lv_obj_align(caption, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_flag(caption, LV_OBJ_FLAG_EVENT_BUBBLE);

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

/** Advances the splash frame after the animation interval elapses. */
void splashTick() {
    if (!splashActive) return;
    uint32_t now = millis();
    if (now - lastFrameMs < kSplashFrameIntervalMs) return;
    lastFrameMs = now;
    frameIndex = static_cast<uint8_t>((frameIndex + 1) % kSplashSweepFrameCount);
    renderFrame();
}

/** Advances the splash frame immediately for button-driven cycling. */
void splashNext() {
    frameIndex = static_cast<uint8_t>((frameIndex + 1) % kSplashSweepFrameCount);
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
