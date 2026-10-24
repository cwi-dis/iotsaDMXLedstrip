#include "iotsa.h"
#include "iotsaPixelstrip.h"
#include "iotsaConfigFile.h"

#ifndef NEOPIXEL_PIN
#define NEOPIXEL_PIN 4  // "Normal" pin for NeoPixel
#endif
#ifndef NEOPIXEL_TYPE
#define NEOPIXEL_TYPE (NEO_GRB + NEO_KHZ800)
#endif
#ifndef NEOPIXEL_COUNT
#define NEOPIXEL_COUNT 1 // Default number of pixels
#endif
#ifndef NEOPIXEL_BPP
#define NEOPIXEL_BPP 3  // Default number of colors per pixel
#endif

// Define TESTMODE_PIN if there is a test-mode switch attached
// to that pin. The LEDs will show a looping pattern if that input is high.
// #undef TESTMODE_PIN
#if !defined(TESTMODE_PIN) && !defined(WITHOUT_TESTMODE)
#define TESTMODE_PIN 5
#endif

// Rights, for a strip used for stage effects (see iotsaPixelstrip.h): the
// pixel data is the show itself and open to everyone on the network, as
// Art-Net DMX is anyway. What an operator sets up for a show (gamma, the test
// pattern, and the DMX module's settings) needs "dmx"; how the strip is wired
// (pin, type, count, LEDs per pixel) "config".
template<class Ctx> bool IotsaPixelstripMod::_stripArgs(Ctx& ctx) {
  IotsaContext::RightsScope rights(ctx);
  // Checked before they are used: the framework saves what is in the fields.
  int newCount = count;
  int newBpp = bpp;
  float newGamma = gamma;
  ctx.argsNeedRight("config");
  bool changed = ctx.arg("pin", pin, IotsaContext::Saved);
  changed |= ctx.arg("stripType", stripType, IotsaContext::Saved);
  changed |= ctx.arg("count", newCount, IotsaContext::Saved);
  changed |= ctx.arg("bpp", newBpp, IotsaContext::Saved);
  ctx.argsNeedRight("dmx");
  changed |= ctx.arg("gamma", newGamma, IotsaContext::Saved);
#ifdef TESTMODE_PIN
  changed |= ctx.arg("testmode", testmode, IotsaContext::Saved);
#endif
  if (newCount < 1) ctx.error(400, "count: at least 1");
  if (newBpp < 1 || newBpp > 4) ctx.error(400, "bpp: 1 to 4");
  if (newGamma <= 0) ctx.error(400, "gamma: more than 0");
  if (ctx.failed()) return false;
  count = newCount;
  bpp = newBpp;
  gamma = newGamma;
  return changed;
}

#ifdef IOTSA_WITH_WEB
void IotsaPixelstripMod::formHandler_args(IotsaWebContext& ctx) {
  if (_pixelArgs(ctx)) return;
  bool changed = _stripArgs(ctx);
  if (ctx.failed()) return;
  if (changed) setupStrip();
}

bool IotsaPixelstripMod::_pixelArgs(IotsaWebContext& ctx) {
  if (ctx.argAction("set", IotsaContext::Transient)) {
    // Note: this sets the value for a single LED, not the value for a single NeoPixel (3/4 leds)
    int idx = 0;
    int val = 0;
    ctx.arg("setIndex", idx, IotsaContext::Transient);
    ctx.arg("setValue", val, IotsaContext::Transient);
    if (ctx.failed()) return true;
    if (buffer && idx >= 0 && idx < count*bpp) {
      buffer[idx] = val;
      dmxOutputChanged();
    } else {
      ctx.error("No such LED.");
    }
    return true;
  }
  if (ctx.argAction("clear", IotsaContext::Transient)) {
    if (buffer) {
      memset(buffer, 0, count*bpp);
      dmxOutputChanged();
    }
    return true;
  }
  return false;
}

void IotsaPixelstripMod::formHandler_fields(IotsaWebContext& ctx) {
  if (ctx.includesEveryday()) {
    {
      IotsaWebContext::Form form(ctx, "Set", "Set pixel", "set");
      ctx.showField("LED", "setIndex", 0, IotsaContext::Transient, "a single LED, not a NeoPixel (3 or 4 LEDs)");
      ctx.showField("Value", "setValue", 0, IotsaContext::Transient, "0 to 255");
    }
    ctx.showButton("clear", "Clear all", IotsaContext::Transient);
  }
  if (ctx.includesSettings()) {
    IotsaContext::RightsScope rights(ctx);
    IotsaWebContext::Form form(ctx);
    ctx.argsNeedRight("config");
    ctx.showField("GPIO pin", "pin", pin, IotsaContext::Saved);
    ctx.showField("NeoPixel type", "stripType", stripType, IotsaContext::Saved, "Adafruit_NeoPixel flags, e.g. 82 for NEO_GRB + NEO_KHZ800");
    ctx.showField("Number of NeoPixels", "count", count, IotsaContext::Saved);
    ctx.showField("LEDs per NeoPixel", "bpp", bpp, IotsaContext::Saved, "3 (RGB) or 4 (RGBW)");
    ctx.argsNeedRight("dmx");
    ctx.showField("Gamma", "gamma", gamma, IotsaContext::Saved, "1.0 neutral, 2.2 suggested");
#ifdef TESTMODE_PIN
    ctx.showField("Test mode", "testmode", testmode, IotsaContext::Saved, "0 off, 1 on, 2 follows the switch");
#endif
  }
}
#endif // IOTSA_WITH_WEB

String IotsaPixelstripMod::info() {
  String message = "<p>Built with pixelstrip module. See <a href=\"/pixelstrip\">/pixelstrip</a> to change NeoPixel strip settings, <a href=\"/api/pixelstrip\">/api/pixelstrip</a> and <a href=\"/api/pixels\">/api/pixels</a> for REST API.</p>";
  return message;
}

void IotsaPixelstripMod::setup() {
  configLoad();
  setupStrip();
#ifdef TESTMODE_PIN
  pinMode(TESTMODE_PIN, INPUT);
#endif
}

void IotsaPixelstripMod::setupStrip() {
  IFDEBUG IotsaSerial.printf("setup count=%d bpp=%d pin=%d striptype=%d\n", count, bpp, pin, stripType);
  if (strip) delete strip;
  if (buffer) free(buffer);
  buffer = (uint8_t *)malloc(count*bpp);
  if (buffer == NULL) {
    IotsaSerial.println("No memory");
  } else {
    memset(buffer, 0, count*bpp);
  }
  if (gammaTable) free(gammaTable);
  gammaTable = NULL;
  if (gamma != 0 && gamma != 1) {
    gammaTable = (uint8_t *)malloc(256);
    if (gammaTable == NULL) {
      IotsaSerial.println("No memory");
    }
    for(int i=0; i<256; i++) {
      float gval = powf((float)i/256.0, gamma);
      int ival = (int)(gval * 256);
      if (ival < 0) ival = 0;
      if (ival > 255) ival = 255;
      gammaTable[i] = ival;
    }
  }
  strip = new Adafruit_NeoPixel(count, pin, stripType);
  dmxOutputChanged();
  if (dmx) {
    dmx->setDMXOutputHandler(0, buffer, count*bpp, this);
  }
}

void IotsaPixelstripMod::getHandler(IotsaApiContext& ctx) {
  if (ctx.path() == "/api/pixelstrip") {
    ctx.show("pin", pin);
    ctx.show("stripType", stripType);
    ctx.show("count", count);
    ctx.show("bpp", bpp);
    ctx.show("gamma", gamma);
#ifdef TESTMODE_PIN
    ctx.show("testmode", testmode);
#endif
  } else if (ctx.path() == "/api/pixels") {
    JsonArray data = ctx.reply()["data"].to<JsonArray>();
    if (buffer) {
      for(int i=0; i<count*bpp; i++) {
        data.add(buffer[i]);
      }
    }
  }
}

// /api/pixels: clear, then data (one value per LED) from start on.
void IotsaPixelstripMod::putHandler(IotsaApiContext& ctx) {
  if (ctx.path() == "/api/pixelstrip") {
    bool changed = _stripArgs(ctx);
    if (ctx.failed()) return;
    if (changed) setupStrip();
  } else if (ctx.path() == "/api/pixels") {
    bool clear = false;
    int start = 0;
    ctx.arg("clear", clear, IotsaContext::Transient);
    ctx.arg("start", start, IotsaContext::Transient);
    bool haveData = ctx.argAction("data", IotsaContext::Transient);
    if (ctx.failed()) return;
    if (buffer == NULL) {
      ctx.error(409, "pixelstrip not configured");
      return;
    }
    if (clear) memset(buffer, 0, count*bpp);
    if (haveData) {
      JsonArray data = ctx.request()["data"].as<JsonArray>();
      for(JsonArray::iterator it=data.begin(); it!=data.end(); ++it) {
        if (start < 0 || start >= count*bpp) {
          ctx.error(400, "data: past the last LED");
          break;
        }
        buffer[start++] = it->as<int>();
      }
    }
    dmxOutputChanged();
  }
}

void IotsaPixelstripMod::dmxOutputChanged() {
  if (buffer == NULL) {
    return;
  }
  uint8_t *ptr = buffer;
  IFDEBUG IotsaSerial.println("Show ");
  strip->begin();
  for (int i=0; i < count; i++) {
    uint32_t color = 0;
    for (int b=0; b<bpp; b++) {
      int cval = *ptr++;
      if (gammaTable) cval = gammaTable[cval];
      color = color << 8 | cval;
    }
    IFDEBUG IotsaSerial.print(color, HEX);
    IFDEBUG IotsaSerial.print(' ');
    strip->setPixelColor(i, color);
  }
  strip->show();
  IFDEBUG IotsaSerial.println(" called");
}

void IotsaPixelstripMod::lateSetup() {
  // /pixelstrip (the module page) is auto-registered by this get=true api.setup().
  api.setup("pixelstrip", true, true);
  // /api/pixels is a second data endpoint on the same module -- no separate web
  // page (webPage=false); getHandler/putHandler dispatch on the path.
  api.setup("pixels", true, true, false, false);
  name = "pixelstrip";
}

void IotsaPixelstripMod::configLoad() {
  IotsaConfigFileLoad cf("/config/pixelstrip.cfg");
  cf.get("pin", pin, NEOPIXEL_PIN);
  cf.get("stripType", stripType, NEOPIXEL_TYPE);
  cf.get("count", count, NEOPIXEL_COUNT);
  cf.get("bpp", bpp, NEOPIXEL_BPP);
#ifdef TESTMODE_PIN
  cf.get("testmode", testmode, 2);
#endif
  cf.get("gamma", gamma, 1.0);
}

void IotsaPixelstripMod::configSave() {
  IotsaConfigFileSave cf("/config/pixelstrip.cfg");
  cf.put("pin", pin);
  cf.put("stripType", stripType);
  cf.put("count", count);
  cf.put("bpp", bpp);
  cf.put("gamma", gamma);
#ifdef TESTMODE_PIN
  cf.put("testmode", testmode);
#endif
}

void IotsaPixelstripMod::loop() {
#ifdef TESTMODE_PIN
  bool testRunning = (testmode == 1);
  if (testmode == 2) {
    testRunning = digitalRead(TESTMODE_PIN);
  }
  if (testRunning && buffer != 0) {
    int value = (millis() >> 3) & 0xff;  // Loop intensities every 2 seconds, approximately
    int bits = (millis() >> 11) % ((1 << bpp)-1); // Cycle over which colors to light
    if (bits == 0) bits = ((1 << bpp)-1);
    for (int i=0; i < count; i++) {
      for (int b=0; b < bpp; b++) {
        int thisValue = 0;
        if (bits & (1<<((b+i) % bpp))) {
          thisValue = value;
        }
        buffer[i*bpp+b] = thisValue;
      }
    }
    dmxOutputChanged();
  }
#endif
}
