#include "serial_protocol.h"

#include "firmware_info.h"

#include <ArduinoJson.h>
#include <stdio.h>
#include <string.h>

static constexpr const char *kUsbProtocolName = "neon-meter-usb";
static constexpr int kUsbProtocolVersion = 1;

/** Returns true when a JSON object looks like a provider payload. */
static bool hasProviderPayloadFields(JsonObjectConst object) {
    return !object["providers"].isNull() || !object["s"].isNull() ||
           !object["p"].isNull() || !object["ok"].isNull();
}

/** Returns a JSON string value when present. */
static const char *jsonString(JsonVariantConst value) {
    return value.is<const char *>() ? value.as<const char *>() : nullptr;
}

/** Stores a compact JSON object as the extracted payload body. */
static bool copyPayload(JsonVariantConst payload, SerialProtocolMessage *out) {
    if (!payload.is<JsonObjectConst>()) return false;
    size_t written = serializeJson(payload, out->payload, sizeof(out->payload));
    if (written == 0 || written >= sizeof(out->payload)) return false;
    out->type = SerialProtocolMessagePayload;
    out->valid = true;
    return true;
}

/** Parses one newline-delimited USB serial JSON frame. */
bool parseSerialProtocolLine(const char *line, SerialProtocolMessage *out) {
    if (!line || !out) return false;
    memset(out, 0, sizeof(*out));
    out->type = SerialProtocolMessageIgnored;

    JsonDocument document;
    DeserializationError error = deserializeJson(document, line);
    if (error) return false;

    JsonObjectConst object = document.as<JsonObjectConst>();
    if (object.isNull()) return false;

    const char *type = jsonString(object["type"]);
    const char *command = jsonString(object["cmd"]);
    if ((type && strcmp(type, "hello") == 0) ||
        (command && strcmp(command, "hello") == 0)) {
        out->type = SerialProtocolMessageHello;
        out->valid = true;
        out->payload[0] = '\0';
        return true;
    }

    if (type && strcmp(type, "ping") == 0) {
        out->type = SerialProtocolMessagePing;
        out->valid = true;
        out->payload[0] = '\0';
        return true;
    }

    if (type && strcmp(type, "payload") == 0) {
        return copyPayload(object["payload"].as<JsonVariantConst>(), out);
    }

    if (!type && hasProviderPayloadFields(object)) {
        return copyPayload(document.as<JsonVariantConst>(), out);
    }

    out->valid = true;
    return true;
}

/** Formats the device hello control frame. */
void formatSerialProtocolHello(char *buffer, size_t bufferLength) {
    if (!buffer || bufferLength == 0) return;
    snprintf(buffer, bufferLength,
             "{\"type\":\"hello\",\"protocol\":\"%s\",\"version\":%d,\"device\":\"Neon Meter\",\"firmwareVersion\":\"%s\",\"chipFamily\":\"%s\"}",
             kUsbProtocolName, kUsbProtocolVersion, kFirmwareVersion, kFirmwareChipFamily);
}

/** Formats the successful payload acknowledgement control frame. */
void formatSerialProtocolAck(char *buffer, size_t bufferLength) {
    if (!buffer || bufferLength == 0) return;
    snprintf(buffer, bufferLength, "{\"type\":\"ack\",\"ack\":true}");
}

/** Formats the rejected payload acknowledgement control frame. */
void formatSerialProtocolNack(char *buffer, size_t bufferLength) {
    if (!buffer || bufferLength == 0) return;
    snprintf(buffer, bufferLength, "{\"type\":\"err\",\"err\":true}");
}

/** Formats the payload refresh request control frame. */
void formatSerialProtocolRefreshRequest(char *buffer, size_t bufferLength) {
    if (!buffer || bufferLength == 0) return;
    snprintf(buffer, bufferLength, "{\"type\":\"refresh-requested\"}");
}
