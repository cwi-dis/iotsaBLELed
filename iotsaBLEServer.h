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
typedef const char * UUIDstring;

class IotsaBLEApiProvider {
public:
  typedef const char * UUIDstring;

  virtual ~IotsaBLEApiProvider() {}
  virtual bool bleCharacteristicWriteCallback(UUIDstring charUUID) = 0;
  virtual bool bleCharacteristicReadCallback(UUIDstring charUUID) = 0;

};

class IotsaBLEServiceProvider {
public:
  typedef IotsaBLEApiProvider::UUIDstring UUIDstring;
  
  virtual ~IotsaBLEServiceProvider() {}
  virtual void bleSetup(const char* serviceUUID, IotsaBLEApiProvider *_apiProvider) = 0;
  virtual void bleAddCharacteristic(UUIDstring charUUID, int mask) = 0;
  virtual void bleCharacteristicSet(UUIDstring charUUID, const uint8_t *data, size_t size) = 0;
  virtual void bleCharacteristicGet(UUIDstring charUUID, uint8_t **datap, size_t *sizep) = 0;
  virtual int bleCharacteristicGetInt(UUIDstring charUUID) = 0;

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
  void bleAddCharacteristic(UUIDstring charUUID, int mask);
  void bleCharacteristicSet(UUIDstring charUUID, const uint8_t *data, size_t size);
  void bleCharacteristicGet(UUIDstring charUUID, uint8_t **datap, size_t *sizep);
  int bleCharacteristicGetInt(UUIDstring charUUID);

protected:
  bool getHandler(const char *path, JsonObject& reply);
  bool putHandler(const char *path, const JsonVariant& request, JsonObject& reply);
  void configLoad();
  void configSave();
  void handler();
  IotsaBLEApiProvider *apiProvider;
  String argument;

  BLEServer *bleServer;
  BLEService *bleService;
  int nCharacteristic;
  UUIDstring  *characteristicUUIDs;
  BLECharacteristic **bleCharacteristics;
};

#endif
