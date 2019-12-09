//
// A "Led" server, which allows control over a single NeoPixel (color,
// duration, on/off pattern). The led can be controlled through a web UI or
// through REST calls (and/or, depending on Iotsa compile time options, COAP calls).
// The web interface can be disabled by building iotsa with IOTSA_WITHOUT_WEB.
//
// This is the application that is usually shipped with new iotsa boards.
//

#include "iotsa.h"
#include "iotsaWifi.h"
#include "iotsaLed.h"

// CHANGE: Add application includes and declarations here

#define WITH_OTA    // Enable Over The Air updates from ArduinoIDE. Needs at least 1MB flash.

IotsaApplication application("Iotsa BLE LED Server");
IotsaWifiMod wifiMod(application);

#ifdef WITH_OTA
#include "iotsaOta.h"
IotsaOtaMod otaMod(application);
#endif

#include "iotsaBLEServer.h"
IotsaBLEServerMod bleserverMod(application);

//
// LED module. 
//

#define NEOPIXEL_PIN 16

class IotsaLedControlMod : public IotsaLedMod, public IotsaBLEApiProvider {
public:
  using IotsaLedMod::IotsaLedMod;
  void serverSetup();
  String info();
  bool blePutHandler(UUIDstring charUUID);
  bool bleGetHandler(UUIDstring charUUID);
  void setBLE(IotsaBLEServiceProvider *_ble);
protected:
  bool getHandler(const char *path, JsonObject& reply);
  bool putHandler(const char *path, const JsonVariant& request, JsonObject& reply);
  IotsaBleApiService bleApi;
private:
  void handler();
};

static IotsaBLEServiceProvider::UUIDstring serviceUUID = "3B006387-1226-4A53-9D24-AFA50C0163A3";
static IotsaBLEServiceProvider::UUIDstring rgbUUID = "72BC73F7-9AF2-452D-BBFB-CE4AF53F499A";

bool IotsaLedControlMod::blePutHandler(UUIDstring charUUID) {
  if (charUUID == rgbUUID) {
      uint32_t _rgb = bleApi.characteristicAsInt(rgbUUID);
      set(_rgb, 1000, 0, 0x7fff);
      IFDEBUG IotsaSerial.printf("xxxjack led: wrote %s value 0x%x\n", charUUID, rgb);
      return true;
  }
  IotsaSerial.println("ledControlMod: ble: write unknown uuid");
  return false;
}

bool IotsaLedControlMod::bleGetHandler(UUIDstring charUUID) {
  if (charUUID == rgbUUID) {
      IFDEBUG IotsaSerial.printf("xxxjack led: read %s value 0x%x\n", charUUID, rgb);
      bleApi.characteristicSet(rgbUUID, rgb);
      return true;
  }
  IotsaSerial.println("ledControlMod: ble: read unknown uuid");
  return false;
}

#ifdef IOTSA_WITH_WEB
void
IotsaLedControlMod::handler() {
  // Handles the page that is specific to the Led module, greets the user and
  // optionally stores a new name to greet the next time.
  bool anyChanged = false;
  uint32_t _rgb = 0xffffff;
  if( server->hasArg("rgb")) {
    _rgb = strtol(server->arg("rgb").c_str(), 0, 16);
    anyChanged = true;
  }
  if (anyChanged) set(_rgb, 1000, 0, 0x7fff);
  
  String message = "<html><head><title>Led Server</title></head><body><h1>Led Server</h1>";
  message += "<form method='get'>";
  message += "Color (hex rrggbb): <input type='text' name='rgb'><br>";
  message += "<input type='submit'></form></body></html>";
  server->send(200, "text/html", message);
}

String IotsaLedControlMod::info() {
  // Return some information about this module, for the main page of the web server.
  String rv = "<p>See <a href=\"/led\">/led</a> for setting the LED color.";
  rv += " Or use REST api at <a href='/api/led'>/api/led</a>.";
  rv += " Or use BLE service " + String(serviceUUID) + ".";
  rv += "</p>";
  return rv;
}
#endif // IOTSA_WITH_WEB

bool IotsaLedControlMod::getHandler(const char *path, JsonObject& reply) {
  reply["rgb"] = rgb;
  return true;
}

bool IotsaLedControlMod::putHandler(const char *path, const JsonVariant& request, JsonObject& reply) {
  uint32_t _rgb = request["rgb"]|0xffffff;
  set(_rgb, 1000, 0, 0x7fff);
  return true;
}

void IotsaLedControlMod::serverSetup() {
  // Setup the web server hooks for this module.
#ifdef IOTSA_WITH_WEB
  server->on("/led", std::bind(&IotsaLedControlMod::handler, this));
#endif // IOTSA_WITH_WEB
  api.setup("/api/led", true, true);
  name = "led";
}

void IotsaLedControlMod::setBLE(IotsaBLEServiceProvider *_ble) {
  bleApi.init(_ble);
  bleApi.bleSetup(serviceUUID, this);
  bleApi.addCharacteristic(rgbUUID, IotsaBLEServiceProvider::READ|IotsaBLEServiceProvider::WRITE);
}

IotsaLedControlMod ledMod(application, NEOPIXEL_PIN);

// Standard setup() method, hands off most work to the application framework
void setup(void){
  application.setup();
  application.serverSetup();
  ledMod.setBLE(&bleserverMod);
}
 
// Standard loop() routine, hands off most work to the application framework
void loop(void){
  application.loop();
}

