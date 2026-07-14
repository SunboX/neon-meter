#include <unity.h>

#include "serial_protocol.h"
#include "usage_model.h"

/** Verifies raw provider bundles remain valid serial payload frames. */
void testRawProviderBundleParsesAsPayload(void) {
    SerialProtocolMessage message = {};

    TEST_ASSERT_TRUE(parseSerialProtocolLine(
        "{\"rotationSeconds\":30,\"providers\":[{\"p\":\"chatgpt\",\"s\":42,\"w\":7,\"ok\":true}]}",
        &message));

    TEST_ASSERT_TRUE(message.valid);
    TEST_ASSERT_EQUAL(SerialProtocolMessagePayload, message.type);

    UsageBundle bundle = {};
    TEST_ASSERT_TRUE(parseUsageBundleJson(message.payload, &bundle));
    TEST_ASSERT_EQUAL(1, bundle.count);
    TEST_ASSERT_EQUAL_STRING("chatgpt", bundle.items[0].provider);
    TEST_ASSERT_EQUAL_FLOAT(42.0f, bundle.items[0].primaryPct);
}

/** Verifies typed USB payload frames extract the provider bundle body. */
void testWrappedPayloadParsesAsPayload(void) {
    SerialProtocolMessage message = {};

    TEST_ASSERT_TRUE(parseSerialProtocolLine(
        "{\"type\":\"payload\",\"payload\":{\"rotationSeconds\":5,\"providers\":[{\"p\":\"claude\",\"s\":12,\"w\":3,\"ok\":true}]}}",
        &message));

    TEST_ASSERT_TRUE(message.valid);
    TEST_ASSERT_EQUAL(SerialProtocolMessagePayload, message.type);

    UsageBundle bundle = {};
    TEST_ASSERT_TRUE(parseUsageBundleJson(message.payload, &bundle));
    TEST_ASSERT_EQUAL(5000, bundle.rotationMs);
    TEST_ASSERT_EQUAL_STRING("claude", bundle.items[0].provider);
    TEST_ASSERT_EQUAL_FLOAT(12.0f, bundle.items[0].primaryPct);
}

/** Verifies host hello frames are recognized without becoming payload data. */
void testHelloParsesAsControlFrame(void) {
    SerialProtocolMessage message = {};

    TEST_ASSERT_TRUE(parseSerialProtocolLine(
        "{\"type\":\"hello\",\"protocol\":\"neon-meter-usb\",\"version\":1}",
        &message));

    TEST_ASSERT_TRUE(message.valid);
    TEST_ASSERT_EQUAL(SerialProtocolMessageHello, message.type);
    TEST_ASSERT_EQUAL_STRING("", message.payload);
}

/** Verifies USB heartbeat frames are recognized without becoming payload data. */
void testPingParsesAsControlFrame(void) {
    SerialProtocolMessage message = {};

    TEST_ASSERT_TRUE(parseSerialProtocolLine(
        "{\"type\":\"ping\",\"protocol\":\"neon-meter-usb\",\"version\":1}",
        &message));

    TEST_ASSERT_TRUE(message.valid);
    TEST_ASSERT_EQUAL(SerialProtocolMessagePing, message.type);
    TEST_ASSERT_EQUAL_STRING("", message.payload);
}

/** Verifies serial control frames use documented newline-delimited JSON. */
void testSerialProtocolFormatsControlFrames(void) {
    char buffer[192] = {};

    formatSerialProtocolHello(buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL_STRING(
        "{\"type\":\"hello\",\"protocol\":\"neon-meter-usb\",\"version\":1,\"device\":\"Neon Meter\",\"firmwareVersion\":\"1.0.6\",\"chipFamily\":\"ESP32-S3\"}",
        buffer);

    formatSerialProtocolAck(buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL_STRING("{\"type\":\"ack\",\"ack\":true}", buffer);

    formatSerialProtocolNack(buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL_STRING("{\"type\":\"err\",\"err\":true}", buffer);

    formatSerialProtocolRefreshRequest(buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL_STRING("{\"type\":\"refresh-requested\"}", buffer);
}

/** Runs the serial USB protocol native test suite. */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(testRawProviderBundleParsesAsPayload);
    RUN_TEST(testWrappedPayloadParsesAsPayload);
    RUN_TEST(testHelloParsesAsControlFrame);
    RUN_TEST(testPingParsesAsControlFrame);
    RUN_TEST(testSerialProtocolFormatsControlFrames);
    return UNITY_END();
}
