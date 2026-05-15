#include <unity.h>

#include "ui_footer.h"

#include <stdio.h>

/** Verifies active usage shows real host detail with a playful rate label. */
void testFooterShowsActiveDetailFromPayload(void) {
    UsageData data = {};
    data.valid = true;
    data.ok = true;
    snprintf(data.status, sizeof(data.status), "%s", "allowed");
    snprintf(data.detail, sizeof(data.detail), "%s", "5h 29% / 7d 4%");

    char footer[80];
    formatUsageFooter(&data, 2, footer, sizeof(footer));

    TEST_ASSERT_EQUAL_STRING("Active climb - 5h 29% / 7d 4%", footer);
}

/** Verifies empty payload detail does not fall back to fake activity verbs. */
void testFooterUsesWaitingCopyWithoutDetail(void) {
    UsageData data = {};
    data.valid = true;
    data.ok = true;
    snprintf(data.status, sizeof(data.status), "%s", "ok");

    char footer[80];
    formatUsageFooter(&data, 0, footer, sizeof(footer));

    TEST_ASSERT_EQUAL_STRING("Quiet - waiting for usage", footer);
}

/** Verifies limited provider status overrides the rate label. */
void testFooterShowsLimitedStatusBeforeRateMood(void) {
    UsageData data = {};
    data.valid = true;
    data.ok = true;
    snprintf(data.status, sizeof(data.status), "%s", "limited");
    snprintf(data.detail, sizeof(data.detail), "%s", "5h 82% / 7d 41%");

    char footer[80];
    formatUsageFooter(&data, 3, footer, sizeof(footer));

    TEST_ASSERT_EQUAL_STRING("Limited - 5h 82% / 7d 41%", footer);
}

/** Verifies host fetch failures show an attention state and error detail. */
void testFooterShowsAttentionStateForFailedPayload(void) {
    UsageData data = {};
    data.valid = true;
    data.ok = false;
    snprintf(data.status, sizeof(data.status), "%s", "error");
    snprintf(data.detail, sizeof(data.detail), "%s", "host fetch failed");

    char footer[80];
    formatUsageFooter(&data, 1, footer, sizeof(footer));

    TEST_ASSERT_EQUAL_STRING("Needs attention - host fetch failed", footer);
}

/** Runs the footer formatting native test suite. */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(testFooterShowsActiveDetailFromPayload);
    RUN_TEST(testFooterUsesWaitingCopyWithoutDetail);
    RUN_TEST(testFooterShowsLimitedStatusBeforeRateMood);
    RUN_TEST(testFooterShowsAttentionStateForFailedPayload);
    return UNITY_END();
}
