#pragma once

#include <stddef.h>
#include <stdint.h>

static constexpr size_t kMaxUsageProviders = 2;

/**
 * Parsed provider usage data used by the display and activity-rate tracker.
 */
struct UsageData {
    char provider[16];
    char title[24];
    char primaryLabel[16];
    char secondaryLabel[16];
    float primaryPct;
    int primaryResetMins;
    float secondaryPct;
    int secondaryResetMins;
    bool sessionEnabled;
    char status[24];
    char detail[48];
    bool ok;
    bool valid;
};

/**
 * Parsed provider bundle sent by the host when one or more sources are detected.
 */
struct UsageBundle {
    UsageData items[kMaxUsageProviders];
    size_t count;
    uint32_t rotationMs;
    bool valid;
};

/**
 * Parses a compact provider JSON payload into an Arduino-friendly usage struct.
 */
bool parseUsageJson(const char *json, UsageData *out);

/**
 * Parses either a single compact payload or a host provider bundle.
 */
bool parseUsageBundleJson(const char *json, UsageBundle *out);

/**
 * Returns the default screen title for a provider id.
 */
const char *providerDefaultTitle(const char *provider);

/**
 * Formats reset minutes into the short display text used by the UI.
 */
void formatResetTime(int minutes, char *buffer, size_t bufferLength);

/**
 * Returns the first available provider percentage for activity tracking.
 */
float usageRatePercent(const UsageData &data);
