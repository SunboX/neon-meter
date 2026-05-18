#pragma once

namespace UiGauge {

/**
 * Color buckets for the remaining-capacity usage gauge.
 */
enum class UsageGaugeColor {
    Green,
    Amber,
    Red,
};

/**
 * Returns a provider usage percentage clamped to display bounds.
 */
constexpr float clampUsedPercent(float usedPercent) {
    if (usedPercent < 0.0f) return 0.0f;
    if (usedPercent > 100.0f) return 100.0f;
    return usedPercent;
}

/**
 * Returns the rounded remaining capacity for a consumed usage percentage.
 */
constexpr int remainingPercent(float usedPercent) {
    return static_cast<int>((100.0f - clampUsedPercent(usedPercent)) + 0.5f);
}

/**
 * Returns the gauge color bucket for a consumed usage percentage.
 */
constexpr UsageGaugeColor colorForUsedPercent(float usedPercent) {
    int remaining = remainingPercent(usedPercent);
    if (remaining <= 20) return UsageGaugeColor::Red;
    if (remaining <= 50) return UsageGaugeColor::Amber;
    return UsageGaugeColor::Green;
}

}
