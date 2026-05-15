#include "usage_rate.h"

static constexpr float kRateThresholdNormal = 0.10f;
static constexpr float kRateThresholdActive = 0.20f;
static constexpr float kRateThresholdHeavy = 0.33f;
static constexpr uint32_t kMinWindowMs = 240000UL;

/** Clears the ring buffer indexes without clearing stale sample bytes. */
void UsageRateTracker::reset() {
    sampleCount = 0;
    writeIndex = 0;
}

/** Returns the oldest logical sample in the circular sample array. */
uint8_t UsageRateTracker::oldestIndex() const {
    return (writeIndex + kRingSize - sampleCount) % kRingSize;
}

/** Adds a usage sample and resets history when the provider counter wraps. */
void UsageRateTracker::addSample(uint32_t nowMs, float primaryPct) {
    if (sampleCount > 0) {
        uint8_t latest = (writeIndex + kRingSize - 1) % kRingSize;
        // A sharp drop means the provider reset its window, so old samples are no longer comparable.
        if (primaryPct + 5.0f < samples[latest].percent) {
            reset();
        }
    }

    samples[writeIndex] = {nowMs, primaryPct};
    writeIndex = (writeIndex + 1) % kRingSize;
    if (sampleCount < kRingSize) sampleCount++;
}

/** Converts the recent percentage-per-minute rate into a 0-3 activity group. */
int UsageRateTracker::getGroup() const {
    if (sampleCount < 2) return 0;

    uint8_t oldest = oldestIndex();
    uint8_t latest = (writeIndex + kRingSize - 1) % kRingSize;
    uint32_t elapsedMs = samples[latest].ms - samples[oldest].ms;
    if (elapsedMs < kMinWindowMs) return 0;

    float percentDelta = samples[latest].percent - samples[oldest].percent;
    if (percentDelta < 0.0f) percentDelta = 0.0f;
    float ratePerMinute = percentDelta * 60000.0f / static_cast<float>(elapsedMs);

    if (ratePerMinute < kRateThresholdNormal) return 0;
    if (ratePerMinute < kRateThresholdActive) return 1;
    if (ratePerMinute < kRateThresholdHeavy) return 2;
    return 3;
}
