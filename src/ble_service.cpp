#include "ble_service.h"

#include <Arduino.h>
#include <NimBLEDevice.h>

static constexpr const char *kDeviceName = "Neon Meter";
static constexpr const char *kServiceUuid = "41494d45-7465-7220-0000-000000000001";
static constexpr const char *kRxCharUuid = "41494d45-7465-7220-0000-000000000002";
static constexpr const char *kTxCharUuid = "41494d45-7465-7220-0000-000000000003";
static constexpr const char *kRequestCharUuid = "41494d45-7465-7220-0000-000000000004";
static constexpr size_t kBleBufferSize = 1024;

static NimBLEServer *bleServer = nullptr;
static NimBLECharacteristic *rxChar = nullptr;
static NimBLECharacteristic *txChar = nullptr;
static NimBLECharacteristic *requestChar = nullptr;
static BleState bleState = BleStateInit;
static bool needAdvertise = false;
static char rxBuffer[kBleBufferSize];
static volatile bool dataReady = false;
static volatile bool hasReceivedData = false;
static char addressText[18] = "---";

/** Starts advertising with the documented Neon Meter service UUID. */
static void startAdvertising() {
    NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
    advertising->reset();
    advertising->addServiceUUID(kServiceUuid);
    advertising->enableScanResponse(true);
    advertising->setName(kDeviceName);
    bool started = advertising->start();
    bleState = BleStateAdvertising;
    Serial.printf("BLE advertising: %s\n", started ? "OK" : "FAILED");
}

/**
 * Handles BLE central connect and disconnect events.
 */
class BleServerCallbacks : public NimBLEServerCallbacks {
    /**
     * Marks the service connected when a central attaches.
     */
    void onConnect(NimBLEServer *server, NimBLEConnInfo &info) override {
        (void)server;
        bleState = BleStateConnected;
        Serial.printf("BLE connected: %s\n", info.getAddress().toString().c_str());
    }

    /**
     * Marks the service disconnected and schedules advertising restart.
     */
    void onDisconnect(NimBLEServer *server, NimBLEConnInfo &info, int reason) override {
        (void)server;
        Serial.printf("BLE disconnected: %s reason=%d\n", info.getAddress().toString().c_str(), reason);
        bleState = BleStateDisconnected;
        needAdvertise = true;
    }
};

/**
 * Handles provider payload writes from the connected host.
 */
class BleRxCallbacks : public NimBLECharacteristicCallbacks {
    /**
     * Copies the written JSON payload into the firmware receive buffer.
     */
    void onWrite(NimBLECharacteristic *characteristic, NimBLEConnInfo &info) override {
        (void)info;
        std::string value = characteristic->getValue();
        size_t length = value.length();
        if (length >= kBleBufferSize) length = kBleBufferSize - 1;
        memcpy(rxBuffer, value.c_str(), length);
        rxBuffer[length] = '\0';
        hasReceivedData = true;
        dataReady = true;
    }
};

/**
 * Handles host subscriptions to the refresh request characteristic.
 */
class BleRequestCallbacks : public NimBLECharacteristicCallbacks {
    /**
     * Requests an initial payload after the host subscribes.
     */
    void onSubscribe(NimBLECharacteristic *characteristic, NimBLEConnInfo &info, uint16_t subValue) override {
        (void)characteristic;
        (void)info;
        if (subValue != 0 && !hasReceivedData) {
            requestBleRefresh();
        }
    }
};

/** Starts NimBLE, creates all service characteristics, and advertises. */
void bleInit() {
    NimBLEDevice::init(kDeviceName);
    NimBLEDevice::setSecurityAuth(true, false, true);

    NimBLEAddress address = NimBLEDevice::getAddress();
    snprintf(addressText, sizeof(addressText), "%s", address.toString().c_str());
    for (int i = 0; addressText[i] != '\0'; i++) {
        if (addressText[i] >= 'a' && addressText[i] <= 'f') {
            addressText[i] = static_cast<char>(addressText[i] - 32);
        }
    }

    bleServer = NimBLEDevice::createServer();
    static BleServerCallbacks serverCallbacks;
    bleServer->setCallbacks(&serverCallbacks);

    NimBLEService *service = bleServer->createService(kServiceUuid);
    rxChar = service->createCharacteristic(kRxCharUuid, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
    static BleRxCallbacks rxCallbacks;
    rxChar->setCallbacks(&rxCallbacks);

    txChar = service->createCharacteristic(kTxCharUuid, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    requestChar = service->createCharacteristic(kRequestCharUuid, NIMBLE_PROPERTY::NOTIFY);
    static BleRequestCallbacks requestCallbacks;
    requestChar->setCallbacks(&requestCallbacks);

    bleServer->start();
    startAdvertising();

    Serial.printf("BLE ready: %s %s\n", kDeviceName, addressText);
}

/** Restarts advertising after disconnect outside the NimBLE callback. */
void bleTick() {
    if (!needAdvertise) return;
    needAdvertise = false;
    startAdvertising();
}

/** Returns the latest cached BLE state. */
BleState getBleState() {
    return bleState;
}

/** Returns the configured BLE device name. */
const char *getBleDeviceName() {
    return kDeviceName;
}

/** Returns the upper-case BLE address string for the UI. */
const char *getBleAddress() {
    return addressText;
}

/** Deletes BLE bonds and disconnects the current central if needed. */
void clearBleBonds() {
    NimBLEDevice::deleteAllBonds();
    Serial.println("BLE bonds cleared");
    if (bleState == BleStateConnected && bleServer && bleServer->getConnectedCount() > 0) {
        bleServer->disconnect(bleServer->getPeerInfo(0).getConnHandle());
    }
    needAdvertise = true;
}

/** Returns true when a pending RX payload has not been consumed. */
bool bleHasData() {
    return dataReady;
}

/** Clears the pending-data flag and returns the RX payload buffer. */
const char *readBleData() {
    dataReady = false;
    return rxBuffer;
}

/** Sends an acknowledgement notification to the host. */
void sendBleAck() {
    if (bleState != BleStateConnected || !txChar) return;
    txChar->setValue("{\"ack\":true}");
    txChar->notify();
}

/** Sends a rejection notification to the host. */
void sendBleNack() {
    if (bleState != BleStateConnected || !txChar) return;
    txChar->setValue("{\"err\":true}");
    txChar->notify();
}

/** Notifies the host that the device needs a fresh payload. */
void requestBleRefresh() {
    if (bleState != BleStateConnected || !requestChar) return;
    // Single-byte notify keeps the refresh signal compatible with the host bridge.
    uint8_t payload = 0x01;
    requestChar->setValue(&payload, 1);
    requestChar->notify();
    Serial.println("BLE refresh requested");
}
