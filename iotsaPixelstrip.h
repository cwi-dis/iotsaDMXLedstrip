#ifndef _IOTSAPIXELSTRIP_H_
#define _IOTSAPIXELSTRIP_H_
#include "iotsa.h"
#include "iotsaApi.h"
#include "iotsaDMX.h"

#ifdef IOTSA_WITH_API
#define IotsaPixelstripModBaseMod IotsaApiMod
#else
#define IotsaPixelstripModBaseMod IotsaMod
#endif

class IotsaPixelstripMod : public IotsaPixelstripModBaseMod, public IotsaDMXHandler {
public:
  IotsaPixelstripMod(IotsaApplication& app)
  : IotsaPixelstripModBaseMod(app),
    dmx(NULL)
  {}
  using IotsaPixelstripModBaseMod::IotsaPixelstripModBaseMod;
  void setup();
  void serverSetup();
  void loop();
  String info();
  void setDMX(IotsaDMXMod *_dmx) { dmx = _dmx; };
  void dmxCallback();
protected:
  bool getHandler(const char *path, JsonObject& reply);
  bool putHandler(const char *path, const JsonVariant& request, JsonObject& reply);
  void configLoad();
  void configSave();
  void handler();
  IotsaDMXMod *dmx;
  String argument;
};

#endif
