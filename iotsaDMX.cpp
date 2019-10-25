#include "iotsa.h"
#include "iotsaDMX.h"
#include "iotsaConfigFile.h"

#define ARTNET_PORT 6454

#ifdef IOTSA_WITH_WEB
void
IotsaDMXMod::handler() {
  bool anyChanged = false;
  if( server->hasArg("shortName")) {
    if (needsAuthentication()) return;
    shortName = server->arg("shortName");
    anyChanged = true;
  }
  if( server->hasArg("longName")) {
    if (needsAuthentication()) return;
    longName = server->arg("longName");
    anyChanged = true;
  }
  if( server->hasArg("portAddress")) {
    if (needsAuthentication()) return;
    portAddress = server->arg("portAddress").toInt();
    anyChanged = true;
  }
  if( server->hasArg("universe")) {
    if (needsAuthentication()) return;
    universe = server->arg("universe").toInt();
    anyChanged = true;
  }
  if( server->hasArg("firstIndex")) {
    if (needsAuthentication()) return;
    firstIndex = server->arg("firstIndex").toInt();
    anyChanged = true;
  }

  if (anyChanged) configSave();

  String message = "<html><head><title>Boilerplate module</title></head><body><h1>Boilerplate module</h1>";
  message += "<form method='get'>Short name: <input name='shortName' value='" + htmlEncode(shortName) + "'><br>";
  message += "Long name: <input name='longName' value='" + htmlEncode(longName) + "'><br>";
  message += "Network, Subnet and Port (16bit int): <input name='portAddress' value='" + String(portAddress) + "'><br>";
  message += "Universe: <input name='universe' value='" + String(universe) + "'><br>";
  message += "Index of first dimmer in universe: <input name='firstIndex' value='" + String(portAddress) + "'><br>";
  message += "Number of dimmers: " + String(count);
  message += "<br><input type='submit'></form>";
  server->send(200, "text/html", message);
}

String IotsaDMXMod::info() {
  String message = "<p>Built with Art-Net DMX module. See <a href=\"/dmx\">/dmx</a> to change the DMX parameters.</p>";
  return message;
}
#endif // IOTSA_WITH_WEB

void IotsaDMXMod::setup() {
  configLoad();
}

#ifdef IOTSA_WITH_API
bool IotsaDMXMod::getHandler(const char *path, JsonObject& reply) {
  reply["shortName"] = shortName;
  reply["longName"] = longName;
  reply["portAddress"] = portAddress;
  reply["universe"] = universe;
  reply["firstIndex"] = firstIndex;
  return true;
}

bool IotsaDMXMod::putHandler(const char *path, const JsonVariant& request, JsonObject& reply) {
  bool anyChanged = false;
  JsonObject& reqObj = request.as<JsonObject>();
  if (reqObj.containsKey("shortName")) {
    shortName = reqObj.get<String>("shortName");
    anyChanged = true;
  }
  if (reqObj.containsKey("longName")) {
    longName = reqObj.get<String>("longName");
    anyChanged = true;
  }
  if (reqObj.containsKey("portAddress")) {
    portAddress = reqObj.get<int>("portAddress");
    anyChanged = true;
  }
  if (reqObj.containsKey("universe")) {
    universe = reqObj.get<int>("universe");
    anyChanged = true;
  }
  if (reqObj.containsKey("firstIndex")) {
    firstIndex = reqObj.get<int>("firstIndex");
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
  udp.begin(ARTNET_PORT);
}

void IotsaDMXMod::configLoad() {
  IotsaConfigFileLoad cf("/config/dmx.cfg");
  cf.get("shortName", shortName, "iotsaDMXLedstrip");
  cf.get("longName", longName, "iotsaDMXLedstrip");
  cf.get("portAddress", portAddress, 0);
  cf.get("universe", universe, 0);
  cf.get("firstIndex", firstIndex, 0);
}

void IotsaDMXMod::configSave() {
  IotsaConfigFileSave cf("/config/dmx.cfg");
  cf.put("shortName", shortName);
  cf.put("longName", longName);
  cf.put("portAddress", portAddress);
  cf.put("universe", universe);
  cf.put("firstIndex", firstIndex);
}

void IotsaDMXMod::loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    IFDEBUG IotsaSerial.print("Received DMX packet,size=");
    IFDEBUG IotsaSerial.println(packetSize);
  }
}
