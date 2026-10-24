// Not the same as lissabon's iotsaPixelstrip (cwi-dis/lissabon, lissabonLedstrip),
// although the hardware is much the same. This one is for stage effects: each
// LED feeds a fibre strand (through a 3D-printed adapter), the strands hang at
// the back or front of a stage, and DMX or REST drives effects (rainbow ripples,
// a VU meter beating to the music), many frames a second. lissabon's is ambient
// lighting: the LEDs together give everyday light. So the rights differ: here
// the pixel data is open and show setup needs "dmx"; there the look needs
// "lighttuning".
//
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
  // Two endpoints: /api/pixelstrip (the settings), /api/pixels (the pixel data).
  void getHandler(IotsaApiContext& ctx) override;
  void putHandler(IotsaApiContext& ctx) override;
#ifdef IOTSA_WITH_WEB
  String webTitle() override { return "Pixelstrip"; }
  void formHandler_args(IotsaWebContext& ctx) override;
  void formHandler_fields(IotsaWebContext& ctx) override;
  // "set pixel" and "clear all". Returns true if it was one of those.
  bool _pixelArgs(IotsaWebContext& ctx);
#endif
  // The strip settings, for both contexts. Returns true if one changed.
  template<class Ctx> bool _stripArgs(Ctx& ctx);
  void configLoad() override;
  void configSave() override;
  void setupStrip();
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
