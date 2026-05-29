#include "ui.h"

#include "ble_service.h"
#include "firmware_info.h"
#include "splash.h"
#include "theme.h"
#include "ui_connection_state.h"
#include "ui_footer.h"
#include "ui_gauge.h"
#include "ui_footer_loader.h"
#include "ui_layout.h"

#include <stdio.h>
#include <string.h>

using namespace UiLayout;

static lv_obj_t *usageContainer = nullptr;
static lv_obj_t *bleContainer = nullptr;
static lv_obj_t *waitingContainer = nullptr;
static lv_obj_t *markContainer = nullptr;
static lv_obj_t *batteryLabel = nullptr;
static lv_obj_t *titleLabel = nullptr;
static lv_obj_t *primaryPctLabel = nullptr;
static lv_obj_t *primaryNameLabel = nullptr;
static lv_obj_t *primaryResetLabel = nullptr;
static lv_obj_t *primaryBar = nullptr;
static lv_obj_t *secondaryPctLabel = nullptr;
static lv_obj_t *secondaryNameLabel = nullptr;
static lv_obj_t *secondaryResetLabel = nullptr;
static lv_obj_t *secondaryBar = nullptr;
static lv_obj_t *footerContainer = nullptr;
static lv_obj_t *footerLoader = nullptr;
static lv_obj_t *footerDots[3] = {};
static lv_obj_t *animationLabel = nullptr;
static lv_obj_t *bleStatusLabel = nullptr;
static lv_obj_t *bleDeviceLabel = nullptr;
static lv_obj_t *bleAddressLabel = nullptr;
static lv_obj_t *waitingStatusLabel = nullptr;

static Screen currentScreen = ScreenUsage;
static Screen previousNonSplashScreen = ScreenUsage;
static bool waitingForConnection = false;
static bool usbConnected = false;
static bool batteryAttached = true;
static uint32_t animationLastMs = 0;
static uint8_t footerLoaderFrame = 0;
static UsageData footerUsageData = {};
static int footerRateGroup = 0;

static void applySharedVisibility();

/** Returns the LVGL fill color for a remaining-capacity gauge bucket. */
static lv_color_t gaugeColor(UiGauge::UsageGaugeColor color) {
    switch (color) {
    case UiGauge::UsageGaugeColor::Red:
        return kThemeRed;
    case UiGauge::UsageGaugeColor::Amber:
        return kThemeAmber;
    default:
        return kThemeGreen;
    }
}

/** Applies the flat screen background expected by the limited color display. */
static void applyScreenBackdrop(lv_obj_t *object) {
    lv_obj_set_style_bg_color(object, kThemeBg, 0);
    lv_obj_set_style_bg_grad_dir(object, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_opa(object, LV_OPA_COVER, 0);
}

/** Adds the shared cyan header accent line to a screen container. */
static void addHeaderAccent(lv_obj_t *parent) {
    lv_obj_t *topAccent = lv_obj_create(parent);
    lv_obj_set_size(topAccent, kScreenWidth - (2 * kMargin), 2);
    lv_obj_set_pos(topAccent, kMargin, 43);
    lv_obj_set_style_bg_color(topAccent, kThemeAccent, 0);
    lv_obj_set_style_bg_opa(topAccent, LV_OPA_50, 0);
    lv_obj_set_style_border_width(topAccent, 0, 0);
    lv_obj_set_style_radius(topAccent, 0, 0);
    lv_obj_set_style_pad_all(topAccent, 0, 0);
    lv_obj_set_style_shadow_width(topAccent, 12, 0);
    lv_obj_set_style_shadow_color(topAccent, kThemeAccent, 0);
    lv_obj_set_style_shadow_opa(topAccent, LV_OPA_30, 0);
    lv_obj_clear_flag(topAccent, LV_OBJ_FLAG_SCROLLABLE);
}

/** Toggles the splash screen from any non-Bluetooth screen click. */
static void handleGlobalClick(lv_event_t *event) {
    (void)event;
    if (uiCurrentScreen() == ScreenBluetooth) return;
    uiToggleSplash();
}

/** Clears BLE bonds when the reset panel is tapped. */
static void handleBleResetClick(lv_event_t *event) {
    (void)event;
    clearBleBonds();
}

/** Creates a standard firmware panel object. */
static lv_obj_t *makePanel(lv_obj_t *parent, int x, int y, int width, int height) {
    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_set_pos(panel, x, y);
    lv_obj_set_size(panel, width, height);
    lv_obj_set_style_bg_color(panel, kThemePanel, 0);
    lv_obj_set_style_bg_grad_dir(panel, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_70, 0);
    lv_obj_set_style_radius(panel, 6, 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_shadow_width(panel, 10, 0);
    lv_obj_set_style_shadow_color(panel, kThemeAccent, 0);
    lv_obj_set_style_shadow_opa(panel, LV_OPA_10, 0);
    lv_obj_set_style_pad_all(panel, 0, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(panel, LV_OBJ_FLAG_EVENT_BUBBLE);
    return panel;
}

/** Creates an Info screen panel with the lighter usage-card background. */
static lv_obj_t *makeInfoPanel(lv_obj_t *parent, int x, int y, int width, int height) {
    lv_obj_t *panel = makePanel(parent, x, y, width, height);
    lv_obj_set_style_bg_color(panel, kThemeUsagePanel, 0);
    return panel;
}

/** Creates a themed percentage bar. */
static lv_obj_t *makeBar(lv_obj_t *parent, int x, int y, int width, int height) {
    lv_obj_t *bar = lv_bar_create(parent);
    lv_obj_set_pos(bar, x, y);
    lv_obj_set_size(bar, width, height);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar, kThemeBarBg, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 4, LV_PART_MAIN);
    lv_obj_set_style_border_width(bar, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, kThemeGreen, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar, 4, LV_PART_INDICATOR);
    lv_obj_set_style_shadow_width(bar, 8, LV_PART_INDICATOR);
    lv_obj_set_style_shadow_color(bar, kThemeGreen, LV_PART_INDICATOR);
    lv_obj_set_style_shadow_opa(bar, LV_OPA_30, LV_PART_INDICATOR);
    return bar;
}

/** Creates the right-aligned provider period label. */
static lv_obj_t *makePill(lv_obj_t *parent, const char *text) {
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_width(label, 106);
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label, kThemeAccent, 0);
    lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(label, 0, 0);
    return label;
}

/** Creates one static arc stroke in the compact Neon Meter mark. */
static lv_obj_t *makeBrandArc(lv_obj_t *parent, int arcWidth, lv_color_t color,
                              lv_opa_t opacity, int endAngle) {
    lv_obj_t *arc = lv_arc_create(parent);
    lv_obj_remove_style_all(arc);
    lv_obj_set_size(arc, kBrandMarkGaugeSize, kBrandMarkGaugeSize);
    lv_obj_set_pos(arc, kBrandMarkGaugeX, kBrandMarkGaugeY);
    lv_obj_set_style_arc_width(arc, arcWidth, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, color, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc, opacity, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(arc, true, LV_PART_MAIN);
    lv_arc_set_bg_angles(arc, kBrandMarkArcStartAngle, endAngle);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(arc, LV_OBJ_FLAG_EVENT_BUBBLE);
    return arc;
}

/** Creates one non-interactive shape in the compact Neon Meter mark. */
static lv_obj_t *makeBrandShape(lv_obj_t *parent, int x, int y, int width, int height,
                                lv_color_t color, lv_opa_t opacity, int radius) {
    lv_obj_t *shape = lv_obj_create(parent);
    lv_obj_remove_style_all(shape);
    lv_obj_set_size(shape, width, height);
    lv_obj_set_pos(shape, x, y);
    lv_obj_set_style_bg_color(shape, color, 0);
    lv_obj_set_style_bg_opa(shape, opacity, 0);
    lv_obj_set_style_radius(shape, radius, 0);
    lv_obj_set_style_border_width(shape, 0, 0);
    lv_obj_set_style_pad_all(shape, 0, 0);
    lv_obj_clear_flag(shape, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(shape, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(shape, LV_OBJ_FLAG_EVENT_BUBBLE);
    return shape;
}

/** Creates one usage percentage panel and returns its child widgets. */
static void makeUsagePanel(lv_obj_t *parent, int y, const char *label,
                           lv_obj_t **percentLabel, lv_obj_t **pillLabel, lv_obj_t **progressBar,
                           lv_obj_t **resetLabel) {
    lv_obj_t *panel = makePanel(parent, kMargin, y, kPanelWidth, kPanelHeight);
    lv_obj_set_style_bg_color(panel, kThemeUsagePanel, 0);

    *percentLabel = lv_label_create(panel);
    lv_label_set_text(*percentLabel, "--%");
    lv_obj_set_width(*percentLabel, 112);
    lv_label_set_long_mode(*percentLabel, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_font(*percentLabel, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(*percentLabel, kThemeText, 0);
    lv_obj_set_pos(*percentLabel, 24, kPanelPercentTextY);

    *pillLabel = makePill(panel, label);
    lv_obj_align(*pillLabel, LV_ALIGN_TOP_RIGHT, -20, kPanelPillTextY);

    *progressBar = makeBar(panel, kPanelBarX, kPanelBarY, kPanelBarWidth, 10);

    *resetLabel = lv_label_create(panel);
    lv_label_set_text(*resetLabel, "---");
    lv_obj_set_width(*resetLabel, kPanelWidth - 48);
    lv_label_set_long_mode(*resetLabel, LV_LABEL_LONG_DOT);
    lv_obj_set_style_text_font(*resetLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(*resetLabel, kThemeDim, 0);
    lv_obj_set_pos(*resetLabel, 24, kResetTextY);
}

/** Creates the compact host-style brand mark. */
static void makeBrandMark(lv_obj_t *screenObject) {
    markContainer = lv_obj_create(screenObject);
    lv_obj_set_size(markContainer, kBrandMarkWidth, kBrandMarkHeight);
    lv_obj_set_pos(markContainer, kMargin, kTitleY + kBrandMarkOffsetY);
    lv_obj_set_style_bg_color(markContainer, kThemeBg2, 0);
    lv_obj_set_style_bg_opa(markContainer, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(markContainer, 6, 0);
    lv_obj_set_style_border_width(markContainer, kBrandMarkBorderWidth, 0);
    lv_obj_set_style_shadow_width(markContainer, 8, 0);
    lv_obj_set_style_shadow_color(markContainer, kThemeAccent, 0);
    lv_obj_set_style_shadow_opa(markContainer, LV_OPA_20, 0);
    lv_obj_set_style_pad_all(markContainer, 0, 0);
    lv_obj_clear_flag(markContainer, LV_OBJ_FLAG_SCROLLABLE);

    makeBrandArc(markContainer, kBrandMarkArcWidth, kThemeIconTrack, LV_OPA_COVER,
                 kBrandMarkArcEndAngle);
    makeBrandArc(markContainer, kBrandMarkAccentArcWidth, kThemeAccent, LV_OPA_COVER,
                 kBrandMarkAccentArcEndAngle);

    makeBrandShape(markContainer, kBrandMarkBaseX, kBrandMarkBaseY, kBrandMarkBaseWidth,
                   kBrandMarkBaseHeight, kThemeIconBase, LV_OPA_COVER, LV_RADIUS_CIRCLE);
    lv_obj_t *needle = makeBrandShape(markContainer, kBrandMarkNeedleX, kBrandMarkNeedleY,
                                      kBrandMarkNeedleWidth, kBrandMarkNeedleHeight,
                                      kThemeAccent, LV_OPA_COVER, LV_RADIUS_CIRCLE);
    lv_obj_set_style_shadow_width(needle, 8, 0);
    lv_obj_set_style_shadow_color(needle, kThemeAccent, 0);
    lv_obj_set_style_shadow_opa(needle, LV_OPA_50, 0);
    lv_obj_set_style_transform_pivot_x(needle, kBrandMarkNeedleWidth / 2, 0);
    lv_obj_set_style_transform_pivot_y(needle, kBrandMarkNeedleHeight / 2, 0);
    lv_obj_set_style_transform_rotation(needle, kBrandMarkNeedleRotation, 0);

    lv_obj_t *hub = makeBrandShape(markContainer, kBrandMarkHubX, kBrandMarkHubY,
                                   kBrandMarkHubSize, kBrandMarkHubSize, kThemeAccent,
                                   LV_OPA_COVER, LV_RADIUS_CIRCLE);
    lv_obj_set_style_shadow_width(hub, 6, 0);
    lv_obj_set_style_shadow_color(hub, kThemeAccent, 0);
    lv_obj_set_style_shadow_opa(hub, LV_OPA_30, 0);

    int hubCoreOffset = (kBrandMarkHubSize - kBrandMarkHubCoreSize) / 2;
    makeBrandShape(markContainer, kBrandMarkHubX + hubCoreOffset,
                   kBrandMarkHubY + hubCoreOffset, kBrandMarkHubCoreSize,
                   kBrandMarkHubCoreSize, kThemeBg2, LV_OPA_COVER, LV_RADIUS_CIRCLE);
    makeBrandShape(markContainer, kBrandMarkNodeX, kBrandMarkNodeY, kBrandMarkNodeSize,
                   kBrandMarkNodeSize, kThemeOrange, LV_OPA_COVER, LV_RADIUS_CIRCLE);
    makeBrandShape(markContainer, kBrandMarkLeftDotX, kBrandMarkDotY, kBrandMarkDotSize,
                   kBrandMarkDotSize, kThemeIconCyanDot, LV_OPA_80, LV_RADIUS_CIRCLE);
    makeBrandShape(markContainer, kBrandMarkRightDotX, kBrandMarkDotY, kBrandMarkDotSize,
                   kBrandMarkDotSize, kThemeIconOrangeDot, LV_OPA_80, LV_RADIUS_CIRCLE);
}

/** Returns the opacity for one footer loader dot in the current frame. */
/** Applies the current brightness frame to the footer loader dots. */
static void updateFooterLoader() {
    for (uint8_t i = 0; i < kFooterLoaderDotCount; ++i) {
        if (!footerDots[i]) continue;
        lv_opa_t opacity = static_cast<lv_opa_t>(footerLoaderDotOpacity(footerLoaderFrame, i));
        lv_obj_set_style_bg_opa(footerDots[i], opacity, 0);
    }
}

/** Creates the animated dot loader and data-backed footer label. */
static void makeFooter(lv_obj_t *parent) {
    footerContainer = lv_obj_create(parent);
    lv_obj_remove_style_all(footerContainer);
    lv_obj_set_size(footerContainer, kPanelWidth, kFooterHeight);
    lv_obj_align(footerContainer, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_layout(footerContainer, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(footerContainer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(footerContainer, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(footerContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(footerContainer, 0, 0);
    lv_obj_set_style_pad_all(footerContainer, 0, 0);
    lv_obj_set_style_pad_column(footerContainer, kFooterLoaderTextGap, 0);
    lv_obj_clear_flag(footerContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(footerContainer, LV_OBJ_FLAG_EVENT_BUBBLE);

    footerLoader = lv_obj_create(footerContainer);
    lv_obj_remove_style_all(footerLoader);
    lv_obj_set_size(footerLoader, kFooterLoaderWidth, kFooterLoaderHeight);
    lv_obj_set_style_bg_opa(footerLoader, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(footerLoader, 0, 0);
    lv_obj_set_style_pad_all(footerLoader, 0, 0);
    lv_obj_clear_flag(footerLoader, LV_OBJ_FLAG_SCROLLABLE);

    for (uint8_t i = 0; i < kFooterLoaderDotCount; ++i) {
        footerDots[i] = lv_obj_create(footerLoader);
        lv_obj_remove_style_all(footerDots[i]);
        lv_obj_set_size(footerDots[i], kFooterLoaderDotSize, kFooterLoaderDotSize);
        lv_obj_set_pos(footerDots[i], i * (kFooterLoaderDotSize + kFooterLoaderDotGap),
                       ((kFooterLoaderHeight - kFooterLoaderDotSize) / 2) + kFooterLoaderDotOffsetY);
        lv_obj_set_style_bg_color(footerDots[i], kThemeOrange, 0);
        lv_obj_set_style_radius(footerDots[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(footerDots[i], 0, 0);
        lv_obj_set_style_shadow_width(footerDots[i], kFooterLoaderDotShadowWidth, 0);
        lv_obj_set_style_pad_all(footerDots[i], 0, 0);
        lv_obj_clear_flag(footerDots[i], LV_OBJ_FLAG_SCROLLABLE);
    }

    animationLabel = lv_label_create(footerContainer);
    lv_label_set_text(animationLabel, "Waiting - waiting for usage");
    lv_label_set_long_mode(animationLabel, LV_LABEL_LONG_DOT);
    lv_obj_set_style_max_width(animationLabel, kFooterTextMaxWidth, 0);
    lv_obj_set_style_text_font(animationLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(animationLabel, kThemeOrange, 0);
    lv_obj_set_style_text_align(animationLabel, LV_TEXT_ALIGN_LEFT, 0);

    updateFooterLoader();
}

/** Creates the shared brand mark and battery widgets. */
static void initSharedHeader(lv_obj_t *screenObject) {
    makeBrandMark(screenObject);

    batteryLabel = lv_label_create(screenObject);
    lv_label_set_text(batteryLabel, "--%");
    lv_obj_set_style_text_font(batteryLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(batteryLabel, kThemeAccent, 0);
    lv_obj_align(batteryLabel, LV_ALIGN_TOP_RIGHT, -kMargin, kTitleY + 8);
}

/** Creates the main provider usage screen. */
static void initUsageScreen(lv_obj_t *screenObject) {
    usageContainer = lv_obj_create(screenObject);
    lv_obj_set_size(usageContainer, kScreenWidth, kScreenHeight);
    lv_obj_set_pos(usageContainer, 0, 0);
    lv_obj_set_style_bg_opa(usageContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(usageContainer, 0, 0);
    lv_obj_set_style_pad_all(usageContainer, 0, 0);
    lv_obj_clear_flag(usageContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(usageContainer, handleGlobalClick, LV_EVENT_CLICKED, nullptr);
    addHeaderAccent(usageContainer);

    titleLabel = lv_label_create(usageContainer);
    lv_label_set_text(titleLabel, "Usage");
    lv_obj_set_width(titleLabel, 176);
    lv_label_set_long_mode(titleLabel, LV_LABEL_LONG_DOT);
    lv_obj_set_style_text_align(titleLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(titleLabel, kThemeAccent, 0);
    lv_obj_align(titleLabel, LV_ALIGN_TOP_MID, 8, kTitleY - 3);

    makeUsagePanel(usageContainer, kContentY, "Current", &primaryPctLabel,
                     &primaryNameLabel, &primaryBar, &primaryResetLabel);
    makeUsagePanel(usageContainer, kContentY + kPanelHeight + kPanelGap, "Weekly", &secondaryPctLabel,
                     &secondaryNameLabel, &secondaryBar, &secondaryResetLabel);

    makeFooter(usageContainer);
}

/** Creates the Bluetooth status and bond-reset screen. */
static void initBluetoothScreen(lv_obj_t *screenObject) {
    bleContainer = lv_obj_create(screenObject);
    lv_obj_set_size(bleContainer, kScreenWidth, kScreenHeight);
    lv_obj_set_pos(bleContainer, 0, 0);
    lv_obj_set_style_bg_opa(bleContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(bleContainer, 0, 0);
    lv_obj_set_style_pad_all(bleContainer, 0, 0);
    lv_obj_clear_flag(bleContainer, LV_OBJ_FLAG_SCROLLABLE);
    addHeaderAccent(bleContainer);

    lv_obj_t *bleTitle = lv_label_create(bleContainer);
    lv_label_set_text(bleTitle, kInfoScreenTitle);
    lv_obj_set_style_text_font(bleTitle, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(bleTitle, kThemeAccent, 0);
    lv_obj_align(bleTitle, LV_ALIGN_TOP_MID, 12, kTitleY);

    lv_obj_t *info = makeInfoPanel(bleContainer, kMargin, kContentY, kPanelWidth, 86);

    bleStatusLabel = lv_label_create(info);
    lv_label_set_text(bleStatusLabel, "Initializing");
    lv_obj_set_style_text_font(bleStatusLabel, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(bleStatusLabel, kThemeDim, 0);
    lv_obj_set_pos(bleStatusLabel, kInfoStatusTextX, kInfoStatusTextY);

    bleDeviceLabel = lv_label_create(info);
    lv_label_set_text(bleDeviceLabel, "Device: ---");
    lv_obj_set_style_text_font(bleDeviceLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(bleDeviceLabel, kThemeDim, 0);
    lv_obj_set_pos(bleDeviceLabel, kInfoStatusTextX, kInfoDeviceTextY);

    bleAddressLabel = lv_label_create(info);
    lv_label_set_text(bleAddressLabel, "Address: ---");
    lv_obj_set_style_text_font(bleAddressLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(bleAddressLabel, kThemeDim, 0);
    lv_obj_set_pos(bleAddressLabel, kInfoStatusTextX, kInfoAddressTextY);

    lv_obj_t *reset = makeInfoPanel(bleContainer, kMargin, kContentY + 96, kPanelWidth, 54);
    lv_obj_set_style_shadow_color(reset, kThemeOrange, 0);
    lv_obj_add_event_cb(reset, handleBleResetClick, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *resetLabel = lv_label_create(reset);
    lv_label_set_text(resetLabel, "Reset Bluetooth");
    lv_obj_set_style_text_font(resetLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(resetLabel, kThemeOrange, 0);
    lv_obj_center(resetLabel);

    lv_obj_t *credit = lv_label_create(bleContainer);
    lv_label_set_text(credit, kInfoFooterText);
    lv_obj_set_style_text_font(credit, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(credit, kThemeDim, 0);
    lv_obj_align(credit, LV_ALIGN_BOTTOM_MID, 0, -8);

    lv_obj_add_flag(bleContainer, LV_OBJ_FLAG_HIDDEN);
}

/** Creates the blocking waiting screen shown until BLE connects. */
static void initWaitingScreen(lv_obj_t *screenObject) {
    waitingContainer = lv_obj_create(screenObject);
    lv_obj_set_size(waitingContainer, kScreenWidth, kScreenHeight);
    lv_obj_set_pos(waitingContainer, 0, 0);
    applyScreenBackdrop(waitingContainer);
    lv_obj_set_style_border_width(waitingContainer, 0, 0);
    lv_obj_set_style_pad_all(waitingContainer, 0, 0);
    lv_obj_clear_flag(waitingContainer, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(waitingContainer);
    lv_label_set_text(title, "Waiting for connection");
    lv_obj_set_width(title, kScreenWidth - 40);
    lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(title, kThemeAccent, 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -28);

    waitingStatusLabel = lv_label_create(waitingContainer);
    lv_label_set_text(waitingStatusLabel, "Bluetooth starting");
    lv_obj_set_width(waitingStatusLabel, kScreenWidth - 40);
    lv_label_set_long_mode(waitingStatusLabel, LV_LABEL_LONG_DOT);
    lv_obj_set_style_text_align(waitingStatusLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(waitingStatusLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(waitingStatusLabel, kThemeDim, 0);
    lv_obj_align(waitingStatusLabel, LV_ALIGN_CENTER, 0, 14);

    lv_obj_add_flag(waitingContainer, LV_OBJ_FLAG_HIDDEN);
}

/** Initializes all UI screens and the shared LVGL event hooks. */
void uiInit() {
    lv_obj_t *screenObject = lv_screen_active();
    applyScreenBackdrop(screenObject);

    initUsageScreen(screenObject);
    initBluetoothScreen(screenObject);
    splashInit(screenObject);
    initWaitingScreen(screenObject);
    initSharedHeader(screenObject);

    if (getSplashRoot()) {
        lv_obj_add_event_cb(getSplashRoot(), handleGlobalClick, LV_EVENT_CLICKED, nullptr);
    }
}

/** Applies the current data-backed footer text. */
static void updateUsageFooter() {
    char buffer[80];
    formatUsageFooter(&footerUsageData, footerRateGroup, buffer, sizeof(buffer));
    lv_label_set_text(animationLabel, buffer);
}

/** Applies parsed provider usage data to labels, bars, and reset text. */
void uiUpdate(const UsageData *data, int rateGroup) {
    if (!data || !data->valid) return;

    footerUsageData = *data;
    footerRateGroup = rateGroup;
    lv_label_set_text(titleLabel, data->title);
    lv_label_set_text(primaryNameLabel, data->primaryLabel);
    lv_label_set_text(secondaryNameLabel, data->secondaryLabel);

    int primary = UiGauge::remainingPercent(data->primaryPct);
    int secondary = UiGauge::remainingPercent(data->secondaryPct);
    lv_color_t primaryGaugeColor = gaugeColor(UiGauge::colorForUsedPercent(data->primaryPct));
    lv_color_t secondaryGaugeColor = gaugeColor(UiGauge::colorForUsedPercent(data->secondaryPct));

    lv_label_set_text_fmt(primaryPctLabel, "%d%%", primary);
    lv_bar_set_value(primaryBar, primary, LV_ANIM_ON);
    lv_obj_set_style_bg_color(primaryBar, primaryGaugeColor, LV_PART_INDICATOR);
    lv_obj_set_style_shadow_color(primaryBar, primaryGaugeColor, LV_PART_INDICATOR);

    char buffer[48];
    formatResetTime(data->primaryResetMins, buffer, sizeof(buffer));
    lv_label_set_text(primaryResetLabel, buffer);

    lv_label_set_text_fmt(secondaryPctLabel, "%d%%", secondary);
    lv_bar_set_value(secondaryBar, secondary, LV_ANIM_ON);
    lv_obj_set_style_bg_color(secondaryBar, secondaryGaugeColor, LV_PART_INDICATOR);
    lv_obj_set_style_shadow_color(secondaryBar, secondaryGaugeColor, LV_PART_INDICATOR);

    formatResetTime(data->secondaryResetMins, buffer, sizeof(buffer));
    lv_label_set_text(secondaryResetLabel, buffer);

    updateUsageFooter();
}

/** Updates the waiting screen text from the active device transport state. */
static void updateWaitingStatus(BleState state) {
    if (!waitingStatusLabel) return;
    if (usbConnected) {
        lv_label_set_text(waitingStatusLabel, "USB connected");
        lv_obj_set_style_text_color(waitingStatusLabel, kThemeGreen, 0);
        return;
    }
    switch (state) {
    case BleStateAdvertising:
        lv_label_set_text(waitingStatusLabel, "Bluetooth advertising");
        lv_obj_set_style_text_color(waitingStatusLabel, kThemeAmber, 0);
        break;
    case BleStateDisconnected:
        lv_label_set_text(waitingStatusLabel, "Bluetooth disconnected");
        lv_obj_set_style_text_color(waitingStatusLabel, kThemeRed, 0);
        break;
    default:
        lv_label_set_text(waitingStatusLabel, "Bluetooth starting");
        lv_obj_set_style_text_color(waitingStatusLabel, kThemeDim, 0);
        break;
    }
}

/** Shows or hides the waiting screen and refreshes shared header visibility. */
static void setWaitingVisible(bool visible) {
    waitingForConnection = visible;
    if (waitingContainer) {
        if (visible) lv_obj_clear_flag(waitingContainer, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(waitingContainer, LV_OBJ_FLAG_HIDDEN);
    }
    applySharedVisibility();
}

/** Advances the usage footer dot loader. */
void uiTickAnimation() {
    if (currentScreen != ScreenUsage) return;
    uint32_t now = lv_tick_get();
    if (now - animationLastMs < 220) return;
    animationLastMs = now;
    footerLoaderFrame = static_cast<uint8_t>((footerLoaderFrame + 1) % kFooterLoaderFrameCount);
    updateFooterLoader();
}

/** Applies BLE state to the Bluetooth screen and waiting overlay. */
void uiUpdateBleStatus(BleState state, const char *name, const char *address) {
    switch (state) {
    case BleStateConnected:
        lv_label_set_text(bleStatusLabel, "Connected");
        lv_obj_set_style_text_color(bleStatusLabel, kThemeGreen, 0);
        break;
    case BleStateAdvertising:
        lv_label_set_text(bleStatusLabel, kBleAdvertisingStatusText);
        lv_obj_set_style_text_color(bleStatusLabel, kThemeAmber, 0);
        break;
    case BleStateDisconnected:
        lv_label_set_text(bleStatusLabel, "Disconnected");
        lv_obj_set_style_text_color(bleStatusLabel, kThemeRed, 0);
        break;
    default:
        lv_label_set_text(bleStatusLabel, "Initializing");
        lv_obj_set_style_text_color(bleStatusLabel, kThemeDim, 0);
        break;
    }

    if (name) lv_label_set_text_fmt(bleDeviceLabel, "Device: %s", name);
    if (address) lv_label_set_text_fmt(bleAddressLabel, "Address: %s", address);

    updateWaitingStatus(state);
    splashSetStatus(state, usbConnected);
    setWaitingVisible(shouldShowWaitingForConnection(state, usbConnected));
}

/** Applies USB protocol state to the waiting overlay. */
void uiUpdateUsbStatus(bool connected) {
    usbConnected = connected;
    BleState state = getBleState();
    updateWaitingStatus(state);
    splashSetStatus(state, usbConnected);
    setWaitingVisible(shouldShowWaitingForConnection(state, usbConnected));
}

/** Applies battery state and attachment visibility to the shared header label. */
void uiUpdateBattery(int percent, bool charging, bool attached) {
    if (!batteryLabel) return;
    batteryAttached = attached;
    applySharedVisibility();
    if (!attached) {
        lv_label_set_text(batteryLabel, "");
        return;
    }
    if (percent < 0) {
        lv_label_set_text(batteryLabel, charging ? "USB" : "--%");
    } else {
        lv_label_set_text_fmt(batteryLabel, charging ? "%d%%+" : "%d%%", percent);
    }
}

/** Hides shared header elements behind overlays that should own the full screen. */
static void applySharedVisibility() {
    bool showSharedHeader = sharedHeaderIsVisible(currentScreen == ScreenSplash, waitingForConnection);
    bool hide = !showSharedHeader;
    if (markContainer) {
        if (hide) lv_obj_add_flag(markContainer, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_clear_flag(markContainer, LV_OBJ_FLAG_HIDDEN);
    }
    if (batteryLabel) {
        if (!batteryHeaderIsVisible(showSharedHeader, batteryAttached)) {
            lv_obj_add_flag(batteryLabel, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(batteryLabel, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

/** Shows the requested top-level screen and hides the others. */
void uiShowScreen(Screen screen) {
    lv_obj_add_flag(usageContainer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bleContainer, LV_OBJ_FLAG_HIDDEN);
    splashHide();

    switch (screen) {
    case ScreenSplash:
        splashShow();
        break;
    case ScreenUsage:
        lv_obj_clear_flag(usageContainer, LV_OBJ_FLAG_HIDDEN);
        break;
    case ScreenBluetooth:
        lv_obj_clear_flag(bleContainer, LV_OBJ_FLAG_HIDDEN);
        break;
    }

    if (screen != ScreenSplash) previousNonSplashScreen = screen;
    currentScreen = screen;
    applySharedVisibility();
}

/** Toggles between the usage and Bluetooth screens. */
void uiCycleScreen() {
    if (currentScreen == ScreenUsage) uiShowScreen(ScreenBluetooth);
    else uiShowScreen(ScreenUsage);
}

/** Toggles the splash screen over the last non-splash screen. */
void uiToggleSplash() {
    if (currentScreen == ScreenSplash) uiShowScreen(previousNonSplashScreen);
    else uiShowScreen(ScreenSplash);
}

/** Returns the active top-level UI screen. */
Screen uiCurrentScreen() {
    return currentScreen;
}
