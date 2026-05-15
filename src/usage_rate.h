#pragma once

#include <stdint.h>

/**
 * Tracks recent usage samples and classifies how quickly usage is increasing.
 */
class UsageRateTracker {
public:
    /**
     * Adds a percentage sample from millis() and resets when usage drops sharply.
     */
    void addSample(uint32_t nowMs, float primaryPct);

    /**
     * Returns a 0-3 activity group based on recent percentage growth.
     */
    int getGroup() const;

    /**
     * Clears all stored samples.
     */
    void reset();

private:
    /**
     * One timestamped percentage sample in the activity-rate ring buffer.
     */
    struct Sample {
        uint32_t ms;
        float percent;
    };

    static constexpr uint8_t kRingSize = 6;
    Sample samples[kRingSize] = {};
    uint8_t sampleCount = 0;
    uint8_t writeIndex = 0;

    /**
     * Returns the oldest valid sample index in the ring buffer.
     */
    uint8_t oldestIndex() const;
};
