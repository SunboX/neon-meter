#include <unity.h>

#include <fstream>
#include <sstream>
#include <string>

/** Reads one project file from the PlatformIO test working directory. */
static std::string readProjectFile(const char *path) {
    std::ifstream file(path);
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/** Counts non-overlapping substring occurrences in source text. */
static int countOccurrences(const std::string &source, const std::string &needle) {
    int count = 0;
    std::size_t position = 0;
    while ((position = source.find(needle, position)) != std::string::npos) {
        count += 1;
        position += needle.length();
    }
    return count;
}

/** Verifies usage cards have their own lighter background token. */
void testUsagePanelHasDedicatedLighterBackgroundToken(void) {
    std::string source = readProjectFile("src/theme.h");

    TEST_ASSERT_TRUE(source.find("#define kThemeUsagePanel lv_color_hex(0x0d1a28)") !=
                     std::string::npos);
}

/** Verifies progress-bar tracks use the lighter requested background token. */
void testProgressBarTrackUsesLighterBackgroundToken(void) {
    std::string source = readProjectFile("src/theme.h");

    TEST_ASSERT_TRUE(source.find("#define kThemeBarBg lv_color_hex(0x102638)") !=
                     std::string::npos);
}

/** Verifies firmware usage panels apply the dedicated card background. */
void testUsagePanelAppliesDedicatedBackgroundToken(void) {
    std::string source = readProjectFile("src/ui.cpp");

    TEST_ASSERT_TRUE(source.find("lv_obj_set_style_bg_color(panel, kThemeUsagePanel, 0);") !=
                     std::string::npos);
}

/** Verifies Info view boxes reuse the lighter card background token. */
void testInfoViewPanelsUseDedicatedBackgroundToken(void) {
    std::string source = readProjectFile("src/ui.cpp");

    TEST_ASSERT_TRUE(source.find("static lv_obj_t *makeInfoPanel") != std::string::npos);
    TEST_ASSERT_TRUE(source.find("lv_obj_set_style_bg_color(panel, kThemeUsagePanel, 0);") !=
                     std::string::npos);
    TEST_ASSERT_EQUAL(2, countOccurrences(source, "makeInfoPanel(bleContainer"));
}

/** Verifies README previews render usage cards with the same background token. */
void testScreenshotRendererUsesDedicatedUsagePanelToken(void) {
    std::string source = readProjectFile("scripts/render_readme_screenshots.mjs");

    TEST_ASSERT_TRUE(source.find("previewUsagePanel: theme.kThemeUsagePanel") !=
                     std::string::npos);
    TEST_ASSERT_GREATER_OR_EQUAL(3, countOccurrences(source, "fill: theme.previewUsagePanel"));
}

/** Runs the UI theme token native test suite. */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(testUsagePanelHasDedicatedLighterBackgroundToken);
    RUN_TEST(testProgressBarTrackUsesLighterBackgroundToken);
    RUN_TEST(testUsagePanelAppliesDedicatedBackgroundToken);
    RUN_TEST(testInfoViewPanelsUseDedicatedBackgroundToken);
    RUN_TEST(testScreenshotRendererUsesDedicatedUsagePanelToken);
    return UNITY_END();
}
