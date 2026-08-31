#ifndef _IOTSAPIXELSTRIP_H_
#define _IOTSAPIXELSTRIP_H_
#include "iotsa.h"
#include "iotsaApi.h"
#include "iotsaDMX.h"
#include <Adafruit_NeoPixel.h>

class IotsaPixelstripMod : public IotsaModule, public IotsaDMXOutputHandler {
public:
  IotsaPixelstripMod(IotsaApplication& app)
  : IotsaModule(app),
    dmx(NULL),
    strip(NULL),
    buffer(NULL),
    gammaTable(NULL),
    testmode(0)
  {}
  void setup() override;
  void lateSetup() override;
  void loop() override;
  String info() override;
  void setDMX(IotsaDMXMod *_dmx) { dmx = _dmx; };
  void dmxOutputChanged() override;
protected:
  bool getHandler(const char *path, JsonObject& reply) override;
  bool putHandler(const char *path, const JsonVariant& request, JsonObject& reply) override;
  void configLoad() override;
  void configSave() override;
  void setupStrip();
  void webHandler() override;
  IotsaDMXMod *dmx;
  Adafruit_NeoPixel *strip;
  uint8_t *buffer;
  int bpp;
  int count;
  int stripType;
  int pin;
  float gamma;
  uint8_t *gammaTable;
  int testmode;
};

#endif
