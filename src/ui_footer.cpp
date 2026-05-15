#include "ui_footer.h"

#include <stdio.h>
#include <string.h>

/** Returns true when two short status tokens are equal. */
static bool statusEquals(const char *left, const char *right) {
    return left && right && strcmp(left, right) == 0;
}

/** Returns the playful footer mood for the current payload and rate group. */
static const char *footerMood(const UsageData *data, int rateGroup) {
    if (!data || !data->valid) return "Waiting";
    if (!data->ok || statusEquals(data->status, "error")) return "Needs attention";
    if (statusEquals(data->status, "limited")) return "Limited";

    switch (rateGroup) {
    case 1:
        return "Warming up";
    case 2:
        return "Active climb";
    case 3:
        return "Heavy burn";
    default:
        return "Quiet";
    }
}

/** Returns the payload detail or a compact fallback when detail is absent. */
static const char *footerDetail(const UsageData *data) {
    if (!data || !data->valid || data->detail[0] == '\0') return "waiting for usage";
    return data->detail;
}

/** Formats the usage footer from real payload status, detail, and rate data. */
void formatUsageFooter(const UsageData *data, int rateGroup, char *buffer, size_t bufferLength) {
    if (!buffer || bufferLength == 0) return;
    snprintf(buffer, bufferLength, "%s - %s", footerMood(data, rateGroup), footerDetail(data));
}
