#ifndef _IOTSABLESERVER_H_
#define _IOTSABLESERVER_H_
#include "iotsa.h"
#include "iotsaApi.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#ifdef IOTSA_WITH_API
#define IotsaBLEServerModBaseMod IotsaApiMod
#else
#define IotsaBLEServerModBaseMod IotsaMod
#endif
class IotsaBLEApiProvider {
public:
  virtual ~IotsaBLEApiProvider() {}
  virtual bool bleCharacteristicWriteCallback(const char *charUUID) = 0;
  virtual bool bleCharacteristicReadCallback(const char *charUUID) = 0;

};

class IotsaBLEServiceProvider {
public:
  virtual ~IotsaBLEServiceProvider() {}
  virtual void bleSetup(const char* serviceUUID, IotsaBLEApiProvider *_apiProvider) = 0;
  virtual void bleAddCharacteristic(const char *charUUID, int mask) = 0;
  virtual void bleCharacteristicSet(const char *charUUID, const uint8_t *data, size_t size) = 0;
  virtual void bleCharacteristicGet(const char *charUUID, uint8_t **datap, size_t *sizep) = 0;
  virtual int bleCharacteristicGetInt(const char *charUUID) = 0;

  static const uint32_t READ = BLECharacteristic::PROPERTY_READ;
  static const uint32_t WRITE = BLECharacteristic::PROPERTY_WRITE;
  static const uint32_t NOTIFY = BLECharacteristic::PROPERTY_NOTIFY;
};

class IotsaBLEServerMod : public IotsaBLEServerModBaseMod, public IotsaBLEServiceProvider {
public:
  using IotsaBLEServerModBaseMod::IotsaBLEServerModBaseMod;
  void setup();
  void serverSetup();
  void loop();
  String info();
  void bleSetup(const char* serviceUUID, IotsaBLEApiProvider *_apiProvider);
  void bleAddCharacteristic(const char *charUUID, int mask);
  void bleCharacteristicSet(const char *charUUID, const uint8_t *data, size_t size);
  void bleCharacteristicGet(const char *charUUID, uint8_t **datap, size_t *sizep);
  int bleCharacteristicGetInt(const char *charUUID);

protected:
  bool getHandler(const char *path, JsonObject& reply);
  bool putHandler(const char *path, const JsonVariant& request, JsonObject& reply);
  void configLoad();
  void configSave();
  void handler();
  IotsaBLEApiProvider *apiProvider;
  String argument;
};

#endif
