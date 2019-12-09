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

class IotsaBLEServerMod;

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
  IotsaBleApiService(IotsaBLEServerMod *_mod=NULL)
  : apiProvider(NULL),
    mod(_mod),
    bleService(NULL),
    nCharacteristic(0),
    characteristicUUIDs(NULL),
    bleCharacteristics(NULL)
  {}
  void init(IotsaBLEServerMod *_mod) { mod=_mod; }
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
  IotsaBLEApiProvider *apiProvider;
  IotsaBLEServerMod *mod;
  BLEService *bleService;
  int nCharacteristic;
  UUIDstring  *characteristicUUIDs;
  BLECharacteristic **bleCharacteristics;
};

class IotsaBLEServerMod : public IotsaBLEServerModBaseMod {
  friend class IotsaBleApiService;
public:
  using IotsaBLEServerModBaseMod::IotsaBLEServerModBaseMod;
  void setup();
  void serverSetup();
  void loop();
  String info();

protected:
  bool getHandler(const char *path, JsonObject& reply);
  bool putHandler(const char *path, const JsonVariant& request, JsonObject& reply);
  void configLoad();
  void configSave();
  void handler();

  static void createServer();
  static BLEServer *s_server;
};

#endif
