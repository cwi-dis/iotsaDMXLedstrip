#include "iotsa.h"
#include "iotsaPixelstrip.h"
#include "iotsaConfigFile.h"

#define NEOPIXEL_PIN 4  // "Normal" pin for NeoPixel
#define NEOPIXEL_TYPE (NEO_GRB + NEO_KHZ800)
#define NEOPIXEL_COUNT 1 // Default number of pixels
#define NEOPIXEL_BPP 3  // Default number of colors per pixel
#ifdef IOTSA_WITH_WEB

void
IotsaPixelstripMod::handler() {
  if( server->hasArg("setIndex") && server->hasArg("setValue")) {
    int idx = server->arg("setIndex").toInt();
    int val = server->arg("setValue").toInt();
    if (buffer && idx >= 0 && idx <= count*bpp) {
      buffer[idx] = val;
      dmxCallback();
    }
  } else if (server->hasArg("clear")) {
    if (buffer) {
      memset(buffer, 0, count*bpp);
      dmxCallback();
    }
  } else {
    bool anyChanged = false;
    if( server->hasArg("pin")) {
      if (needsAuthentication()) return;
      pin = server->arg("pin").toInt();
      anyChanged = true;
    }
    if( server->hasArg("stripType")) {
      if (needsAuthentication()) return;
      stripType = server->arg("stripType").toInt();
      anyChanged = true;
    }
    if( server->hasArg("count")) {
      if (needsAuthentication()) return;
      count = server->arg("count").toInt();
      anyChanged = true;
    }
    if( server->hasArg("bpp")) {
      if (needsAuthentication()) return;
      bpp = server->arg("bpp").toInt();
      anyChanged = true;
    }
    if (anyChanged) {
      configSave();
      setupStrip();
    }
  }

  String message = "<html><head><title>Pixelstrip module</title></head><body><h1>Pixelstrip module</h1>";
  message += "<h2>Configuration</h2><form method='get'>GPIO pin: <input name='pin' value='" + String(pin) + "'><br>";
  message += "Neopixel type: <input name='stripType' value='" + String(stripType) + "'><br>";
  message += "Number of NeoPixels: <input name='count' value='" + String(count) + "'><br>";
  message += "LEDs per NeoPixel: <input name='bpp' value='" + String(bpp) + "'><br>";
  message += "<input type='submit'></form>";
  message += "<h2>Set pixel</h2><form method='get'><br>Set pixel <input name='setIndex'> to <input name='setValue'><br>";
  message += "<input type='submit'></form>";
  message += "<h2>Clear All</h2><form method='get'><input type='submit' name='clear'></form>";
  server->send(200, "text/html", message);
}

String IotsaPixelstripMod::info() {
  String message = "<p>Built with pixelstrip module. See <a href=\"/pixelstrip\">/pixelstrip</a> to change NeoPixel strip settings.</p>";
  return message;
}
#endif // IOTSA_WITH_WEB

void IotsaPixelstripMod::setup() {
  configLoad();
  setupStrip();
}

void IotsaPixelstripMod::setupStrip() {
  IFDEBUG IotsaSerial.printf("setup count=%d bpp=%d pin=%d striptype=%d\n", count, bpp, pin, stripType);
  if (strip) delete strip;
  if (buffer) free(buffer);
  buffer = (uint8_t *)malloc(count*bpp);
  if (buffer == NULL) {
    IotsaSerial.println("No memory");
  }
  memset(buffer, 0, count*bpp);
  strip = new Adafruit_NeoPixel(count, pin, stripType);
  dmxCallback();
}

#ifdef IOTSA_WITH_API
bool IotsaPixelstripMod::getHandler(const char *path, JsonObject& reply) {
  reply["pin"] = pin;
  reply["stripType"] = stripType;
  reply["count"] = count;
  reply["bpp"] = bpp;
  return true;
}

bool IotsaPixelstripMod::putHandler(const char *path, const JsonVariant& request, JsonObject& reply) {
  bool anyChanged = false;
  JsonObject& reqObj = request.as<JsonObject>();
  if (reqObj.containsKey("pin")) {
    pin = reqObj.get<int>("pin");
    anyChanged = true;
  }
  if (reqObj.containsKey("stripType")) {
    stripType = reqObj.get<int>("stripType");
    anyChanged = true;
  }
  if (reqObj.containsKey("count")) {
    count = reqObj.get<int>("count");
    anyChanged = true;
  }
  if (reqObj.containsKey("bpp")) {
    bpp = reqObj.get<int>("bpp");
    anyChanged = true;
  }
  if (anyChanged) {
    configSave();
    setupStrip();
  }
  return anyChanged;
}
#endif // IOTSA_WITH_API

void IotsaPixelstripMod::dmxCallback() {
  if (buffer == NULL) {
    return;
  }
  uint8_t *ptr = buffer;
  IFDEBUG IotsaSerial.println("Show ");
  strip->begin();
  for (int i=0; i < count; i++) {
    uint32_t color = 0;
    for (int b=0; b<bpp; b++) {
      color = color << 8 | *ptr++;
    }
    IFDEBUG IotsaSerial.print(color);
    IFDEBUG IotsaSerial.print(' ');
    strip->setPixelColor(i, color);
  }
  strip->show();
  IFDEBUG IotsaSerial.println(" called");
}

void IotsaPixelstripMod::serverSetup() {
#ifdef IOTSA_WITH_WEB
  server->on("/pixelstrip", std::bind(&IotsaPixelstripMod::handler, this));
#endif
#ifdef IOTSA_WITH_API
  api.setup("/api/pixelstrip", true, true);
  name = "pixelstrip";
#endif
}

void IotsaPixelstripMod::configLoad() {
  IotsaConfigFileLoad cf("/config/pixelstrip.cfg");
  cf.get("pin", pin, NEOPIXEL_PIN);
  cf.get("stripType", stripType, NEOPIXEL_TYPE);
  cf.get("count", count, NEOPIXEL_COUNT);
  cf.get("bpp", bpp, NEOPIXEL_BPP);
}

void IotsaPixelstripMod::configSave() {
  IotsaConfigFileSave cf("/config/pixelstrip.cfg");
  cf.put("pin", pin);
  cf.put("stripType", stripType);
  cf.put("count", count);
  cf.put("bpp", bpp);
}

void IotsaPixelstripMod::loop() {
}
