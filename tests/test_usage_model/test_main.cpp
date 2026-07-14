#include <unity.h>

#include "usage_model.h"
#include "usage_rate.h"

/** Verifies legacy compact payloads keep Claude-compatible defaults. */
void testClawdmeterPayloadDefaultsToClaude(void) {
    UsageData data = {};

    TEST_ASSERT_TRUE(parseUsageJson(
        "{\"s\":29,\"sr\":142,\"w\":4,\"wr\":9730,\"st\":\"allowed\",\"ok\":true}",
        &data));

    TEST_ASSERT_TRUE(data.valid);
    TEST_ASSERT_EQUAL_STRING("claude", data.provider);
    TEST_ASSERT_EQUAL_STRING("Usage", data.title);
    TEST_ASSERT_EQUAL_STRING("Current", data.primaryLabel);
    TEST_ASSERT_EQUAL_STRING("Weekly", data.secondaryLabel);
    TEST_ASSERT_EQUAL_FLOAT(29.0f, data.primaryPct);
    TEST_ASSERT_EQUAL(142, data.primaryResetMins);
    TEST_ASSERT_EQUAL_FLOAT(4.0f, data.secondaryPct);
    TEST_ASSERT_EQUAL(9730, data.secondaryResetMins);
    TEST_ASSERT_TRUE(data.sessionEnabled);
    TEST_ASSERT_EQUAL_STRING("allowed", data.status);
    TEST_ASSERT_TRUE(data.ok);
}

/** Verifies a missing Session remains absent while Weekly data is preserved. */
void testUsageParserKeepsOptionalSessionState(void) {
    UsageData data = {};

    TEST_ASSERT_TRUE(parseUsageJson(
        "{\"p\":\"chatgpt\",\"se\":false,\"s\":0,\"sr\":-1,"
        "\"w\":52,\"wl\":\"Weekly\",\"wr\":7942,\"detail\":\"7d 52%\",\"ok\":true}",
        &data));

    TEST_ASSERT_FALSE(data.sessionEnabled);
    TEST_ASSERT_EQUAL_FLOAT(52.0f, data.secondaryPct);
    TEST_ASSERT_EQUAL_FLOAT(52.0f, usageRatePercent(data));
}

/** Verifies provider metadata and detail text survive parsing. */
void testChatGptPayloadKeepsProviderLabelsAndDetail(void) {
    UsageData data = {};

    TEST_ASSERT_TRUE(parseUsageJson(
        "{\"p\":\"chatgpt\",\"title\":\"ChatGPT\",\"s\":37,\"sl\":\"Session\",\"sr\":510,"
        "\"w\":62,\"wl\":\"Weekly\",\"wr\":19840,\"st\":\"ok\","
        "\"detail\":\"5h 37% / 7d 62%\",\"ok\":true}",
        &data));

    TEST_ASSERT_EQUAL_STRING("chatgpt", data.provider);
    TEST_ASSERT_EQUAL_STRING("ChatGPT", data.title);
    TEST_ASSERT_EQUAL_STRING("Session", data.primaryLabel);
    TEST_ASSERT_EQUAL_STRING("Weekly", data.secondaryLabel);
    TEST_ASSERT_EQUAL_FLOAT(37.0f, data.primaryPct);
    TEST_ASSERT_EQUAL_FLOAT(62.0f, data.secondaryPct);
    TEST_ASSERT_EQUAL_STRING("5h 37% / 7d 62%", data.detail);
}

/** Verifies unknown provider titles use the product name fallback. */
void testUnknownProviderDefaultsToNeonMeterTitle(void) {
    UsageData data = {};

    TEST_ASSERT_TRUE(parseUsageJson(
        "{\"p\":\"local\",\"s\":12,\"w\":3,\"ok\":true}",
        &data));

    TEST_ASSERT_EQUAL_STRING("local", data.provider);
    TEST_ASSERT_EQUAL_STRING("Neon Meter", data.title);
}

/** Verifies multi-provider host envelopes parse into a rotating bundle. */
void testProviderBundleParsesMultipleProviders(void) {
    UsageBundle bundle = {};

    TEST_ASSERT_TRUE(parseUsageBundleJson(
        "{\"rotationSeconds\":30,\"providers\":["
        "{\"p\":\"claude\",\"title\":\"Claude Code\",\"s\":22,\"sl\":\"Session\",\"sr\":120,"
        "\"w\":8,\"wl\":\"Weekly\",\"wr\":3000,\"st\":\"allowed\",\"detail\":\"5h 22%\",\"ok\":true},"
        "{\"p\":\"chatgpt\",\"title\":\"ChatGPT\",\"s\":44,\"sl\":\"Session\",\"sr\":240,"
        "\"w\":16,\"wl\":\"Weekly\",\"wr\":4000,\"st\":\"ok\",\"detail\":\"5h 44%\",\"ok\":true}"
        "]}",
        &bundle));

    TEST_ASSERT_TRUE(bundle.valid);
    TEST_ASSERT_EQUAL(2, bundle.count);
    TEST_ASSERT_EQUAL(30000, bundle.rotationMs);
    TEST_ASSERT_EQUAL_STRING("claude", bundle.items[0].provider);
    TEST_ASSERT_EQUAL_STRING("Claude Code", bundle.items[0].title);
    TEST_ASSERT_EQUAL_FLOAT(22.0f, bundle.items[0].primaryPct);
    TEST_ASSERT_EQUAL_STRING("chatgpt", bundle.items[1].provider);
    TEST_ASSERT_EQUAL_STRING("ChatGPT", bundle.items[1].title);
    TEST_ASSERT_EQUAL_FLOAT(44.0f, bundle.items[1].primaryPct);
}

/** Verifies a single compact payload remains a one-item bundle. */
void testSinglePayloadParsesAsOneItemBundle(void) {
    UsageBundle bundle = {};

    TEST_ASSERT_TRUE(parseUsageBundleJson(
        "{\"p\":\"claude\",\"title\":\"Claude Code\",\"s\":29,\"sr\":142,\"w\":4,\"wr\":9730,\"ok\":true}",
        &bundle));

    TEST_ASSERT_TRUE(bundle.valid);
    TEST_ASSERT_EQUAL(1, bundle.count);
    TEST_ASSERT_EQUAL(30000, bundle.rotationMs);
    TEST_ASSERT_EQUAL_STRING("claude", bundle.items[0].provider);
    TEST_ASSERT_EQUAL_STRING("Claude Code", bundle.items[0].title);
}

/** Verifies parser percentage clamping at the display bounds. */
void testUsageParserClampsPercentages(void) {
    UsageData data = {};

    TEST_ASSERT_TRUE(parseUsageJson(
        "{\"p\":\"chatgpt\",\"s\":-12,\"w\":140,\"ok\":false}",
        &data));

    TEST_ASSERT_EQUAL_FLOAT(0.0f, data.primaryPct);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, data.secondaryPct);
    TEST_ASSERT_FALSE(data.ok);
}

/** Verifies reset-due windows show fresh usage instead of stale exhaustion. */
void testUsageParserClearsExpiredResetWindows(void) {
    UsageData data = {};

    TEST_ASSERT_TRUE(parseUsageJson(
        "{\"p\":\"chatgpt\",\"s\":100,\"sr\":0,\"w\":87,\"wr\":5,\"ok\":true}",
        &data));

    TEST_ASSERT_EQUAL_FLOAT(0.0f, data.primaryPct);
    TEST_ASSERT_EQUAL_FLOAT(87.0f, data.secondaryPct);

    TEST_ASSERT_TRUE(parseUsageJson(
        "{\"p\":\"chatgpt\",\"s\":23,\"sr\":5,\"w\":100,\"wr\":0,\"ok\":true}",
        &data));

    TEST_ASSERT_EQUAL_FLOAT(23.0f, data.primaryPct);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, data.secondaryPct);
}

/** Verifies activity groups rise after a stable sampling window. */
void testUsageRateGroupsAfterStableWindow(void) {
    UsageRateTracker tracker;

    tracker.addSample(0, 10.0f);
    tracker.addSample(60000, 10.1f);
    tracker.addSample(120000, 10.2f);
    TEST_ASSERT_EQUAL(0, tracker.getGroup());

    tracker.addSample(240000, 10.6f);
    TEST_ASSERT_EQUAL(1, tracker.getGroup());

    tracker.addSample(300000, 11.2f);
    TEST_ASSERT_EQUAL(2, tracker.getGroup());

    tracker.addSample(360000, 12.2f);
    TEST_ASSERT_EQUAL(3, tracker.getGroup());
}

/** Verifies sharp percentage drops reset the activity-rate window. */
void testUsageRateResetsWhenPercentDrops(void) {
    UsageRateTracker tracker;

    tracker.addSample(0, 80.0f);
    tracker.addSample(300000, 95.0f);
    TEST_ASSERT_EQUAL(3, tracker.getGroup());

    tracker.addSample(360000, 10.0f);
    TEST_ASSERT_EQUAL(0, tracker.getGroup());
}

/** Runs the usage model and rate tracker native test suite. */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(testClawdmeterPayloadDefaultsToClaude);
    RUN_TEST(testChatGptPayloadKeepsProviderLabelsAndDetail);
    RUN_TEST(testUnknownProviderDefaultsToNeonMeterTitle);
    RUN_TEST(testProviderBundleParsesMultipleProviders);
    RUN_TEST(testSinglePayloadParsesAsOneItemBundle);
    RUN_TEST(testUsageParserClampsPercentages);
    RUN_TEST(testUsageParserClearsExpiredResetWindows);
    RUN_TEST(testUsageParserKeepsOptionalSessionState);
    RUN_TEST(testUsageRateGroupsAfterStableWindow);
    RUN_TEST(testUsageRateResetsWhenPercentDrops);
    return UNITY_END();
}
