#pragma once

#include <stddef.h>

static constexpr size_t kSerialProtocolPayloadSize = 1024;

/**
 * USB serial frame kinds understood by the firmware.
 */
enum SerialProtocolMessageType {
    SerialProtocolMessageIgnored = 0,
    SerialProtocolMessageHello,
    SerialProtocolMessagePing,
    SerialProtocolMessagePayload
};

/**
 * Parsed USB serial protocol frame.
 */
struct SerialProtocolMessage {
    SerialProtocolMessageType type;
    char payload[kSerialProtocolPayloadSize];
    bool valid;
};

/**
 * Parses one newline-delimited USB serial JSON frame.
 */
bool parseSerialProtocolLine(const char *line, SerialProtocolMessage *out);

/**
 * Formats the device hello control frame.
 */
void formatSerialProtocolHello(char *buffer, size_t bufferLength);

/**
 * Formats the successful payload acknowledgement control frame.
 */
void formatSerialProtocolAck(char *buffer, size_t bufferLength);

/**
 * Formats the rejected payload acknowledgement control frame.
 */
void formatSerialProtocolNack(char *buffer, size_t bufferLength);

/**
 * Formats the payload refresh request control frame.
 */
void formatSerialProtocolRefreshRequest(char *buffer, size_t bufferLength);
