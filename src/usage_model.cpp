#include "usage_model.h"

#include <ArduinoJson.h>
#include <stdio.h>
#include <string.h>

static constexpr uint32_t kDefaultRotationMs = 30000;
static constexpr uint32_t kMinRotationMs = 5000;
static constexpr uint32_t kMaxRotationMs = 3600000;

/** Keeps provider percentages within the displayable 0-100 range. */
static float clampPercent(float value) {
    if (value < 0.0f) return 0.0f;
    if (value > 100.0f) return 100.0f;
    return value;
}

/** Copies a JSON string or fallback into a fixed Arduino display buffer. */
static void copyField(char *destination, size_t destinationLength, const char *value, const char *fallback) {
    if (!destination || destinationLength == 0) return;
    const char *source = (value && value[0] != '\0') ? value : fallback;
    if (!source) source = "";
    snprintf(destination, destinationLength, "%s", source);
}

/** Returns a JSON string only when the variant actually stores one. */
static const char *jsonString(JsonVariantConst value) {
    return value.is<const char *>() ? value.as<const char *>() : nullptr;
}

/** Returns a compact default title for known provider ids. */
const char *providerDefaultTitle(const char *provider) {
    if (!provider || provider[0] == '\0') return "Usage";
    if (strcmp(provider, "host") == 0) return "Neon Meter";
    if (strcmp(provider, "chatgpt") == 0) return "ChatGPT";
    if (strcmp(provider, "claude") == 0) return "Usage";
    return "Neon Meter";
}

/** Parses one compact payload object into a UsageData record. */
static bool parseUsageObject(JsonVariantConst payload, UsageData *out) {
    if (!out || !payload.is<JsonObjectConst>()) return false;
    memset(out, 0, sizeof(*out));

    // Clawdmeter-compatible payloads omit provider metadata, so keep Claude as the legacy fallback.
    const char *provider = jsonString(payload["p"]);
    if (!provider) provider = "claude";
    copyField(out->provider, sizeof(out->provider), provider, "claude");
    copyField(out->title, sizeof(out->title), jsonString(payload["title"]),
              providerDefaultTitle(out->provider));
    copyField(out->primaryLabel, sizeof(out->primaryLabel), jsonString(payload["sl"]), "Current");
    copyField(out->secondaryLabel, sizeof(out->secondaryLabel), jsonString(payload["wl"]), "Weekly");
    copyField(out->status, sizeof(out->status), jsonString(payload["st"]), "unknown");
    copyField(out->detail, sizeof(out->detail), jsonString(payload["detail"]), "");

    out->primaryPct = clampPercent(payload["s"] | 0.0f);
    out->primaryResetMins = payload["sr"] | -1;
    out->secondaryPct = clampPercent(payload["w"] | 0.0f);
    out->secondaryResetMins = payload["wr"] | -1;
    out->ok = payload["ok"] | false;
    out->valid = true;
    return true;
}

/** Converts a rotation field to bounded milliseconds. */
static uint32_t parseRotationMs(JsonVariantConst payload) {
    uint32_t seconds = payload["rotationSeconds"] | 30;
    uint32_t rotationMs = seconds * 1000;
    if (rotationMs < kMinRotationMs) return kDefaultRotationMs;
    if (rotationMs > kMaxRotationMs) return kMaxRotationMs;
    return rotationMs;
}

/** Parses compact JSON fields while preserving legacy Clawdmeter defaults. */
bool parseUsageJson(const char *json, UsageData *out) {
    if (!json || !out) return false;

    JsonDocument document;
    DeserializationError error = deserializeJson(document, json);
    if (error) return false;

    return parseUsageObject(document.as<JsonVariantConst>(), out);
}

/** Parses either a provider bundle or a single-provider compact payload. */
bool parseUsageBundleJson(const char *json, UsageBundle *out) {
    if (!json || !out) return false;

    JsonDocument document;
    DeserializationError error = deserializeJson(document, json);
    if (error) return false;

    memset(out, 0, sizeof(*out));
    out->rotationMs = parseRotationMs(document.as<JsonVariantConst>());

    JsonArrayConst providers = document["providers"].as<JsonArrayConst>();
    if (!providers.isNull()) {
        for (JsonVariantConst item : providers) {
            if (out->count >= kMaxUsageProviders) break;
            if (parseUsageObject(item, &out->items[out->count])) {
                out->count++;
            }
        }
    } else if (parseUsageObject(document.as<JsonVariantConst>(), &out->items[0])) {
        out->count = 1;
    }

    out->valid = out->count > 0;
    return out->valid;
}

/** Formats minute offsets into display text for usage reset labels. */
void formatResetTime(int minutes, char *buffer, size_t bufferLength) {
    if (!buffer || bufferLength == 0) return;
    if (minutes < 0) {
        snprintf(buffer, bufferLength, "---");
    } else if (minutes < 60) {
        snprintf(buffer, bufferLength, "Resets in %dm", minutes);
    } else if (minutes < 1440) {
        snprintf(buffer, bufferLength, "Resets in %dh %dm", minutes / 60, minutes % 60);
    } else {
        snprintf(buffer, bufferLength, "Resets in %dd %dh", minutes / 1440, (minutes % 1440) / 60);
    }
}
