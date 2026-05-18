#include "ble_service.h"

#include "firmware_info.h"

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLESecurity.h>
#include <host/ble_store.h>

static constexpr const char *kDeviceName = "Neon Meter";
static constexpr const char *kServiceUuid = "41494d45-7465-7220-0000-000000000001";
static constexpr const char *kRxCharUuid = "41494d45-7465-7220-0000-000000000002";
static constexpr const char *kTxCharUuid = "41494d45-7465-7220-0000-000000000003";
static constexpr const char *kRequestCharUuid = "41494d45-7465-7220-0000-000000000004";
static constexpr const char *kMetadataCharUuid = "41494d45-7465-7220-0000-000000000005";
static constexpr size_t kBleBufferSize = 1024;

static BLEServer *bleServer = nullptr;
static BLECharacteristic *rxChar = nullptr;
static BLECharacteristic *txChar = nullptr;
static BLECharacteristic *requestChar = nullptr;
static BLECharacteristic *metadataChar = nullptr;
static BleState bleState = BleStateInit;
static bool needAdvertise = false;
static char rxBuffer[kBleBufferSize];
static volatile bool dataReady = false;
static volatile bool hasReceivedData = false;
static char addressText[18] = "---";

/** Starts advertising with the documented Neon Meter service UUID. */
static void startAdvertising() {
    BLEAdvertising *advertising = BLEDevice::getAdvertising();
    advertising->reset();
    advertising->addServiceUUID(kServiceUuid);
    advertising->setScanResponse(true);
    advertising->setName(kDeviceName);
    bool started = advertising->start();
    bleState = BleStateAdvertising;
    Serial.printf("BLE advertising: %s\n", started ? "OK" : "FAILED");
}

/**
 * Handles BLE central connect and disconnect events.
 */
class BleServerCallbacks : public BLEServerCallbacks {
    /**
     * Marks the service connected when a central attaches.
     */
    void onConnect(BLEServer *server, ble_gap_conn_desc *desc) override {
        (void)server;
        bleState = BleStateConnected;
        BLEAddress address(desc->peer_id_addr);
        Serial.printf("BLE connected: %s\n", address.toString().c_str());
    }

    /**
     * Marks the service disconnected and schedules advertising restart.
     */
    void onDisconnect(BLEServer *server, ble_gap_conn_desc *desc) override {
        (void)server;
        BLEAddress address(desc->peer_id_addr);
        Serial.printf("BLE disconnected: %s\n", address.toString().c_str());
        bleState = BleStateDisconnected;
        needAdvertise = true;
    }
};

/**
 * Handles provider payload writes from the connected host.
 */
class BleRxCallbacks : public BLECharacteristicCallbacks {
    /**
     * Copies the written JSON payload into the firmware receive buffer.
     */
    void onWrite(BLECharacteristic *characteristic, ble_gap_conn_desc *desc) override {
        (void)desc;
        String value = characteristic->getValue();
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
class BleRequestCallbacks : public BLECharacteristicCallbacks {
    /**
     * Requests an initial payload after the host subscribes.
     */
    void onSubscribe(BLECharacteristic *characteristic, ble_gap_conn_desc *desc, uint16_t subValue) override {
        (void)characteristic;
        (void)desc;
        if (subValue != 0 && !hasReceivedData) {
            requestBleRefresh();
        }
    }
};

/** Starts NimBLE, creates all service characteristics, and advertises. */
void bleInit() {
    BLEDevice::init(kDeviceName);
    BLESecurity::setAuthenticationMode(true, false, true);

    BLEAddress address = BLEDevice::getAddress();
    snprintf(addressText, sizeof(addressText), "%s", address.toString().c_str());
    for (int i = 0; addressText[i] != '\0'; i++) {
        if (addressText[i] >= 'a' && addressText[i] <= 'f') {
            addressText[i] = static_cast<char>(addressText[i] - 32);
        }
    }

    bleServer = BLEDevice::createServer();
    static BleServerCallbacks serverCallbacks;
    bleServer->setCallbacks(&serverCallbacks);

    BLEService *service = bleServer->createService(kServiceUuid);
    rxChar = service->createCharacteristic(kRxCharUuid,
                                           BLECharacteristic::PROPERTY_WRITE |
                                               BLECharacteristic::PROPERTY_WRITE_NR);
    static BleRxCallbacks rxCallbacks;
    rxChar->setCallbacks(&rxCallbacks);

    txChar = service->createCharacteristic(kTxCharUuid,
                                           BLECharacteristic::PROPERTY_READ |
                                               BLECharacteristic::PROPERTY_NOTIFY);

    requestChar = service->createCharacteristic(kRequestCharUuid, BLECharacteristic::PROPERTY_NOTIFY);
    static BleRequestCallbacks requestCallbacks;
    requestChar->setCallbacks(&requestCallbacks);

    metadataChar = service->createCharacteristic(kMetadataCharUuid, BLECharacteristic::PROPERTY_READ);
    char metadata[128] = {};
    formatFirmwareMetadata(metadata, sizeof(metadata));
    metadataChar->setValue(metadata);

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
    ble_store_clear();
    Serial.println("BLE bonds cleared");
    if (bleState == BleStateConnected && bleServer && bleServer->getConnectedCount() > 0) {
        for (const auto &peer : bleServer->getPeerDevices(false)) {
            bleServer->disconnect(peer.first);
        }
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

/** Returns the BLE firmware metadata characteristic UUID. */
const char *getBleMetadataUuid() {
    return kMetadataCharUuid;
}
