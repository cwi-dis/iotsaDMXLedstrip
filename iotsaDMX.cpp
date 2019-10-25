#include "iotsa.h"
#include "iotsaDMX.h"
#include "iotsaConfigFile.h"

#ifdef IOTSA_WITH_WEB
void
IotsaDMXMod::handler() {
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

String IotsaDMXMod::info() {
  String message = "<p>Built with boilerplate module. See <a href=\"/dmx\">/dmx</a> to change the boilerplate module argument.</p>";
  return message;
}
#endif // IOTSA_WITH_WEB

void IotsaDMXMod::setup() {
  configLoad();
}

#ifdef IOTSA_WITH_API
bool IotsaDMXMod::getHandler(const char *path, JsonObject& reply) {
  reply["argument"] = argument;
  return true;
}

bool IotsaDMXMod::putHandler(const char *path, const JsonVariant& request, JsonObject& reply) {
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

void IotsaDMXMod::serverSetup() {
#ifdef IOTSA_WITH_WEB
  server->on("/dmx", std::bind(&IotsaDMXMod::handler, this));
#endif
#ifdef IOTSA_WITH_API
  api.setup("/api/dmx", true, true);
  name = "dmx";
#endif
}

void IotsaDMXMod::configLoad() {
  IotsaConfigFileLoad cf("/config/dmx.cfg");
  cf.get("argument", argument, "");
 
}

void IotsaDMXMod::configSave() {
  IotsaConfigFileSave cf("/config/dmx.cfg");
  cf.put("argument", argument);
}

void IotsaDMXMod::loop() {
}
