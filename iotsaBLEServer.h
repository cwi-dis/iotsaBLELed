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
  virtual bool blePutHandler(UUIDstring charUUID) = 0;
  virtual bool bleGetHandler(UUIDstring charUUID) = 0;

};


class IotsaBLEServiceProvider {
public:
  typedef IotsaBLEApiProvider::UUIDstring UUIDstring;
  
  virtual ~IotsaBLEServiceProvider() {}
  virtual void bleSetup(const char* serviceUUID, IotsaBLEApiProvider *_apiProvider) = 0;
  virtual void addCharacteristic(UUIDstring charUUID, int mask) = 0;
  virtual void characteristicSetFromBuffer(UUIDstring charUUID, const uint8_t *data, size_t size) = 0;
  virtual void characteristicSet(UUIDstring charUUID, uint8_t value) = 0;
  virtual void characteristicSet(UUIDstring charUUID, uint16_t value) = 0;
  virtual void characteristicSet(UUIDstring charUUID, uint32_t value) = 0;
  virtual void characteristicSet(UUIDstring charUUID, const std::string& value) = 0;
  virtual void characteristicSet(UUIDstring charUUID, const String& value) = 0;
  virtual void characteristicAsBuffer(UUIDstring charUUID, uint8_t **datap, size_t *sizep) = 0;
  virtual int characteristicAsInt(UUIDstring charUUID) = 0;
  virtual std::string characteristicAsString(UUIDstring charUUID) = 0;

  static const uint32_t READ = BLECharacteristic::PROPERTY_READ;
  static const uint32_t WRITE = BLECharacteristic::PROPERTY_WRITE;
  static const uint32_t NOTIFY = BLECharacteristic::PROPERTY_NOTIFY;
};

class IotsaBleApiService {
public:
  IotsaBleApiService(IotsaBLEServiceProvider *_mod=NULL)
  : mod(_mod)
  {}
  void init(IotsaBLEServiceProvider *_mod) { mod=_mod; }
  void bleSetup(const char* serviceUUID, IotsaBLEApiProvider *_apiProvider) { return mod->bleSetup(serviceUUID, _apiProvider); }
  void addCharacteristic(UUIDstring charUUID, int mask) { return mod->addCharacteristic(charUUID, mask); }
  void characteristicSetFromBuffer(UUIDstring charUUID, const uint8_t *data, size_t size) { return mod->characteristicSetFromBuffer(charUUID, data, size); }
  void characteristicSet(UUIDstring charUUID, uint8_t value) { return mod->characteristicSet(charUUID, value); }
  void characteristicSet(UUIDstring charUUID, uint16_t value) { return mod->characteristicSet(charUUID, value); }
  void characteristicSet(UUIDstring charUUID, uint32_t value) { return mod->characteristicSet(charUUID, value); }
  void characteristicSet(UUIDstring charUUID, const std::string& value) { return mod->characteristicSet(charUUID, value); }
  void characteristicSet(UUIDstring charUUID, const String& value) { return mod->characteristicSet(charUUID, value); }
  void characteristicAsBuffer(UUIDstring charUUID, uint8_t **datap, size_t *sizep) { return mod->characteristicAsBuffer(charUUID, datap, sizep); }
  int characteristicAsInt(UUIDstring charUUID) { return mod->characteristicAsInt(charUUID); }
  std::string characteristicAsString(UUIDstring charUUID) { return mod->characteristicAsString(charUUID); }
protected:
  IotsaBLEServiceProvider *mod;
};

class IotsaBLEServerMod : public IotsaBLEServerModBaseMod, public IotsaBLEServiceProvider {
public:
  using IotsaBLEServerModBaseMod::IotsaBLEServerModBaseMod;
  void setup();
  void serverSetup();
  void loop();
  String info();
  void bleSetup(const char* serviceUUID, IotsaBLEApiProvider *_apiProvider);
  void addCharacteristic(UUIDstring charUUID, int mask);
  void characteristicSetFromBuffer(UUIDstring charUUID, const uint8_t *data, size_t size);
  void characteristicSet(UUIDstring charUUID, uint8_t value);
  void characteristicSet(UUIDstring charUUID, uint16_t value);
  void characteristicSet(UUIDstring charUUID, uint32_t value);
  void characteristicSet(UUIDstring charUUID, const std::string& value);
  void characteristicSet(UUIDstring charUUID, const String& value);
  void characteristicAsBuffer(UUIDstring charUUID, uint8_t **datap, size_t *sizep);
  int characteristicAsInt(UUIDstring charUUID);
  std::string characteristicAsString(UUIDstring charUUID);

protected:
  bool getHandler(const char *path, JsonObject& reply);
  bool putHandler(const char *path, const JsonVariant& request, JsonObject& reply);
  void configLoad();
  void configSave();
  void handler();
  IotsaBLEApiProvider *apiProvider;

  static void createServer();
  static BLEServer *s_server;
  BLEService *bleService;
  int nCharacteristic;
  UUIDstring  *characteristicUUIDs;
  BLECharacteristic **bleCharacteristics;
};

#endif
