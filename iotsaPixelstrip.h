#ifndef _IOTSAPIXELSTRIP_H_
#define _IOTSAPIXELSTRIP_H_
#include "iotsa.h"
#include "iotsaApi.h"
#include "iotsaDMX.h"
#include <Adafruit_NeoPixel.h>

#ifdef IOTSA_WITH_API
#define IotsaPixelstripModBaseMod IotsaApiMod
#else
#define IotsaPixelstripModBaseMod IotsaMod
#endif

class IotsaPixelstripMod : public IotsaPixelstripModBaseMod, public IotsaDMXOutputHandler {
public:
  IotsaPixelstripMod(IotsaApplication& app)
  : IotsaPixelstripModBaseMod(app),
    dmx(NULL),
    strip(NULL),
    gammaTable(NULL),
    testmode(0)
  {}
  using IotsaPixelstripModBaseMod::IotsaPixelstripModBaseMod;
  void setup() override;
  void serverSetup() override;
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
  void handler();
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
