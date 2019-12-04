#include "iotsa.h"
#include "iotsaBLEServer.h"
#include "iotsaConfigFile.h"

std::string bleDeviceName("iotsa ble server");
std::string bleServiceUUID("b339b14e-9b57-41a1-8b43-14501e57f20a");
std::string bleCharacteristicUUID("9f25a896-a41f-4895-9ed3-645d32e64293");
int bleCharacteristicProperties = BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE;

BLEServer *bleServer;
BLEService *bleService;
BLECharacteristic *bleCharacteristic;

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
  BLEDevice::init(bleDeviceName);
  bleServer = BLEDevice::createServer();
  bleService = bleServer->createService(bleServiceUUID);
  bleCharacteristic = bleService->createCharacteristic(bleCharacteristicUUID, bleCharacteristicProperties);
  bleCharacteristic->setValue((uint8_t *)argument.c_str(), argument.length());
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(bleServiceUUID);
  pAdvertising->setScanResponse(true);
//  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
//  pAdvertising->setMinPreferred(0x12);
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
