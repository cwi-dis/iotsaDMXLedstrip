#include "iotsa.h"
#include "iotsaPixelstrip.h"
#include "iotsaConfigFile.h"

#ifdef IOTSA_WITH_WEB
void
IotsaPixelstripMod::handler() {
  bool anyChanged = false;
  if( server->hasArg("argument")) {
    if (needsAuthentication()) return;
    argument = server->arg("argument");
    anyChanged = true;
  }
  if (anyChanged) configSave();

  String message = "<html><head><title>Boilerplate module</title></head><body><h1>Boilerplate module</h1>";
  message += "<form method='get'>Argument: <input name='argument' value='";
  message += htmlEncode(argument);
  message += "'><br><input type='submit'></form>";
  server->send(200, "text/html", message);
}

String IotsaPixelstripMod::info() {
  String message = "<p>Built with boilerplate module. See <a href=\"/pixelstrip\">/pixelstrip</a> to change the boilerplate module argument.</p>";
  return message;
}
#endif // IOTSA_WITH_WEB

void IotsaPixelstripMod::setup() {
  configLoad();
}

#ifdef IOTSA_WITH_API
bool IotsaPixelstripMod::getHandler(const char *path, JsonObject& reply) {
  reply["argument"] = argument;
  return true;
}

bool IotsaPixelstripMod::putHandler(const char *path, const JsonVariant& request, JsonObject& reply) {
  bool anyChanged = false;
  JsonObject& reqObj = request.as<JsonObject>();
  if (reqObj.containsKey("argument")) {
    argument = reqObj.get<String>("argument");
    anyChanged = true;
  }
  if (anyChanged) configSave();
  return anyChanged;
}
#endif // IOTSA_WITH_API

void IotsaPixelstripMod::dmxCallback() {

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
  cf.get("argument", argument, "");
 
}

void IotsaPixelstripMod::configSave() {
  IotsaConfigFileSave cf("/config/pixelstrip.cfg");
  cf.put("argument", argument);
}

void IotsaPixelstripMod::loop() {
}
