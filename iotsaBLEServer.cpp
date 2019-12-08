#include "iotsa.h"
#include "iotsaBLEServer.h"
#include "iotsaConfigFile.h"

class IotsaBLEServerCallbacks : public BLEServerCallbacks {
	void onConnect(BLEServer* pServer) {
    IFDEBUG IotsaSerial.println("BLE connect\n");
  }
	void onDisconnect(BLEServer* pServer) {
    IFDEBUG IotsaSerial.println("BLE Disconnect\n");

  }

};
class IotsaBLECharacteristicCallbacks : public BLECharacteristicCallbacks {
public:
  IotsaBLECharacteristicCallbacks(UUIDstring _charUUID, IotsaBLEApiProvider *_api)
  : charUUID(_charUUID),
    api(_api)
  {}

	void onRead(BLECharacteristic* pCharacteristic) {
    IFDEBUG IotsaSerial.printf("BLE char onRead 0x%x\n", (uint32_t)pCharacteristic);
    api->bleCharacteristicReadCallback(charUUID);
  }
	void onWrite(BLECharacteristic* pCharacteristic) {
    IFDEBUG IotsaSerial.printf("BLE char onWrite\n");
    api->bleCharacteristicWriteCallback(charUUID);
  }
	void onNotify(BLECharacteristic* pCharacteristic) {
    IFDEBUG IotsaSerial.printf("BLE char onNotify\n");
  }
	void onStatus(BLECharacteristic* pCharacteristic, Status s, uint32_t code) {
    IFDEBUG IotsaSerial.printf("BLE char onStatus\n");
  }
private:
  UUIDstring charUUID;
  IotsaBLEApiProvider *api;
};

#ifdef IOTSA_WITH_WEB
void
IotsaBLEServerMod::handler() {
  bool anyChanged = false;
  if (anyChanged) configSave();
  
  String message = "<html><head><title>BLE Server module</title></head><body><h1>BLE Server module</h1>";
  message += "<p>Nothing to be seen here, yet.</p>";
  server->send(200, "text/html", message);
}

String IotsaBLEServerMod::info() {
  String message = "<p>Built with BLE server module. See <a href=\"/bleserver\">/bleserver</a> to change settings.</p>";
  return message;
}
#endif // IOTSA_WITH_WEB

BLEServer *IotsaBLEServerMod::s_server = 0;

void IotsaBLEServerMod::createServer() {
  if (s_server) return;
  BLEDevice::init(iotsaConfig.hostName.c_str());
  s_server = BLEDevice::createServer();
  s_server->setCallbacks(new IotsaBLEServerCallbacks());
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setScanResponse(true);
  pAdvertising->start();
}

void IotsaBLEServerMod::setup() {
  configLoad();
  createServer();
  bleService = NULL;
  nCharacteristic = 0;
  characteristicUUIDs = NULL;
  bleCharacteristics = NULL;
}

#ifdef IOTSA_WITH_API
bool IotsaBLEServerMod::getHandler(const char *path, JsonObject& reply) {
  return true;
}

bool IotsaBLEServerMod::putHandler(const char *path, const JsonVariant& request, JsonObject& reply) {
  bool anyChanged = false;
  if (anyChanged) configSave();
  return anyChanged;
}
#endif // IOTSA_WITH_API

void IotsaBLEServerMod::serverSetup() {
#ifdef IOTSA_WITH_WEB
  server->on("/bleserver", std::bind(&IotsaBLEServerMod::handler, this));
#endif
#ifdef IOTSA_WITH_API
  api.setup("/api/bleserver", true, true);
  name = "bleserver";
#endif
}

void IotsaBLEServerMod::bleSetup(const char* serviceUUID, IotsaBLEApiProvider *_apiProvider) {
  apiProvider = _apiProvider;
  IFDEBUG IotsaSerial.printf("ble service %s to 0x%x\n", serviceUUID, (uint32_t)apiProvider);
  bleService = s_server->createService(serviceUUID);
  bleService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->stop();
  pAdvertising->addServiceUUID(serviceUUID);
  pAdvertising->start();
}

void IotsaBLEServerMod::bleAddCharacteristic(UUIDstring charUUID, int mask) {
  IFDEBUG IotsaSerial.printf("ble characteristic %s mask %d\n", charUUID, mask);
  nCharacteristic++;
  characteristicUUIDs = (UUIDstring *)realloc((void *)characteristicUUIDs, nCharacteristic*sizeof(UUIDstring));
  bleCharacteristics = (BLECharacteristic **)realloc((void *)bleCharacteristics, nCharacteristic*sizeof(BLECharacteristic *));
  if (characteristicUUIDs == NULL || bleCharacteristics == NULL) {
    IotsaSerial.println("bleAddCharacteristic out of memory");
    return;
  }
  bleService->stop();
  BLECharacteristic *newChar = bleService->createCharacteristic(charUUID, mask);
  newChar->setCallbacks(new IotsaBLECharacteristicCallbacks(charUUID, apiProvider));

  characteristicUUIDs[nCharacteristic-1] = charUUID;
  bleCharacteristics[nCharacteristic-1] = newChar;
  bleService->start();
}

void IotsaBLEServerMod::bleCharacteristicSet(UUIDstring charUUID, const uint8_t *data, size_t size) {
  for(int i=0; i<nCharacteristic; i++) {
    if (characteristicUUIDs[i] == charUUID) {
      bleCharacteristics[i]->setValue((uint8_t *)data, size);
      return;
    }
    IotsaSerial.println("bleCharacteristicSet: unknown characteristic");
  }
}

void IotsaBLEServerMod::bleCharacteristicGet(UUIDstring charUUID, uint8_t **datap, size_t *sizep) {
  for(int i=0; i<nCharacteristic; i++) {
    if (characteristicUUIDs[i] == charUUID) {
      auto value = bleCharacteristics[i]->getValue();
      if (datap) *datap = (uint8_t *)value.c_str();
      if (sizep) *sizep = value.size();
      return;
    }
    IotsaSerial.println("bleCharacteristicSet: unknown characteristic");
  }
}

int IotsaBLEServerMod::bleCharacteristicGetInt(UUIDstring charUUID) {
  size_t size;
  uint8_t *ptr;
  int val = 0;
  bleCharacteristicGet(charUUID, &ptr, &size);
  while (size--) {
    val = (val << 8) | *ptr++;
  }
  return val;
}

void IotsaBLEServerMod::configLoad() {
  IotsaConfigFileLoad cf("/config/bleserver.cfg"); 
}

void IotsaBLEServerMod::configSave() {
  IotsaConfigFileSave cf("/config/bleserver.cfg");
}

void IotsaBLEServerMod::loop() {
}
