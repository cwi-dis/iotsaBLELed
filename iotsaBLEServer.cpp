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
	void onRead(BLECharacteristic* pCharacteristic) {
    IFDEBUG IotsaSerial.printf("BLE char onRead 0x%x\n", (uint32_t)pCharacteristic);
  }
	void onWrite(BLECharacteristic* pCharacteristic) {
    IFDEBUG IotsaSerial.printf("BLE char onWrite\n");
  }
	void onNotify(BLECharacteristic* pCharacteristic) {
    IFDEBUG IotsaSerial.printf("BLE char onNotify\n");
  }
	void onStatus(BLECharacteristic* pCharacteristic, Status s, uint32_t code) {
    IFDEBUG IotsaSerial.printf("BLE char onStatus\n");
  }

};

std::string bleDeviceName("iotsa ble server");
std::string bleServiceUUID("a339b14e-9b57-41a1-8b43-14501e57f20a");
std::string bleServiceUUID2("6e0a4baa-a5d0-40a8-a111-56ed713bb0e1");
std::string bleCharacteristicUUID("9f25a896-a41f-4895-9ed3-645d32e64293");
std::string bleCharacteristic2UUID("9f25a896-a41f-4895-9ed3-645d32e64293");
std::string bleCharacteristic3UUID("9932723c-c295-459d-8b97-0125e7e5f4b4");
int bleCharacteristicProperties = BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE;

BLEServer *bleServer;
BLEService *bleService;
BLEService *bleService2;
BLECharacteristic *bleCharacteristic;
BLECharacteristic *bleCharacteristic2;
BLECharacteristic *bleCharacteristic3;

#ifdef IOTSA_WITH_WEB
void
IotsaBLEServerMod::handler() {
  bool anyChanged = false;
  if( server->hasArg("argument")) {
    if (needsAuthentication()) return;
    argument = server->arg("argument");
    bleCharacteristic->setValue((uint8_t *)argument.c_str(), argument.length());
    anyChanged = true;
  }
  if (anyChanged) configSave();
  
  argument = bleCharacteristic->getValue().c_str();

  String message = "<html><head><title>BLE Server module</title></head><body><h1>BLE Server module</h1>";
  message += "<form method='get'>Argument: <input name='argument' value='";
  message += htmlEncode(argument);
  message += "'><br><input type='submit'></form>";
  server->send(200, "text/html", message);
}

String IotsaBLEServerMod::info() {
  String message = "<p>Built with BLE server module. See <a href=\"/bleserver\">/bleserver</a> to change settings.</p>";
  return message;
}
#endif // IOTSA_WITH_WEB

void IotsaBLEServerMod::setup() {
  configLoad();
  BLEDevice::init(iotsaConfig.hostName.c_str());
  bleServer = BLEDevice::createServer();
  bleServer->setCallbacks(new IotsaBLEServerCallbacks());

  bleService = bleServer->createService(bleServiceUUID);
  
  bleCharacteristic = bleService->createCharacteristic(bleCharacteristicUUID, bleCharacteristicProperties);
  IFDEBUG IotsaSerial.printf("ble char1=0x%x\n", (uint32_t)bleCharacteristic);
  bleCharacteristic->setValue((uint8_t *)argument.c_str(), argument.length());
  bleCharacteristic->setCallbacks(new IotsaBLECharacteristicCallbacks());
  bleService->start();

  bleService2 = bleServer->createService(bleServiceUUID2);

  bleCharacteristic2 = bleService2->createCharacteristic(bleCharacteristic2UUID, bleCharacteristicProperties);
  IFDEBUG IotsaSerial.printf("ble char2=0x%x\n", (uint32_t)bleCharacteristic2);
  bleCharacteristic2->setValue((uint8_t *)"0042", 4);
  bleCharacteristic2->setCallbacks(new IotsaBLECharacteristicCallbacks());

  bleCharacteristic3 = bleService2->createCharacteristic(bleCharacteristic3UUID, bleCharacteristicProperties);
  IFDEBUG IotsaSerial.printf("ble char3=0x%x\n", (uint32_t)bleCharacteristic3);
  bleCharacteristic3->setValue((uint8_t *)"hi", 2);
  bleCharacteristic3->setCallbacks(new IotsaBLECharacteristicCallbacks());
  bleService2->start();
  
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(bleServiceUUID2);  
  pAdvertising->addServiceUUID(bleServiceUUID);
  pAdvertising->setScanResponse(true);
  //pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  //pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

}

#ifdef IOTSA_WITH_API
bool IotsaBLEServerMod::getHandler(const char *path, JsonObject& reply) {
  argument = bleCharacteristic->getValue().c_str();
  reply["argument"] = argument;
  return true;
}

bool IotsaBLEServerMod::putHandler(const char *path, const JsonVariant& request, JsonObject& reply) {
  bool anyChanged = false;
  JsonObject reqObj = request.as<JsonObject>();
  if (reqObj.containsKey("argument")) {
    argument = reqObj["argument"].as<String>();
    bleCharacteristic->setValue((uint8_t *)argument.c_str(), argument.length());
    anyChanged = true;
  }
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

void IotsaBLEServerMod::configLoad() {
  IotsaConfigFileLoad cf("/config/bleserver.cfg");
  cf.get("argument", argument, "");
 
}

void IotsaBLEServerMod::configSave() {
  IotsaConfigFileSave cf("/config/bleserver.cfg");
  cf.put("argument", argument);
}

void IotsaBLEServerMod::loop() {
}
