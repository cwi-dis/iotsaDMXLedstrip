#include "iotsa.h"
#include "iotsaDMX.h"
#include "iotsaConfigFile.h"

#define ARTNET_PORT 6454

struct ArtnetHeader {
  char ident[8];
  uint16_t opcode;
  uint16_t protocolVersion;
};

struct ArtnetPacket {
  struct ArtnetHeader header;
  union {
    struct {
      uint32_t ipaddr;
      uint16_t port;
      uint16_t version;
      uint16_t dmxPort;
      uint16_t oem;
      uint8_t ubea;
      uint16_t esta;
      uint8_t status;
      char shortName[18];
      char longName[64];
      char report[64];
      uint16_t numPorts;
      uint8_t portType[4];
      uint8_t inputStatus[4];
      uint8_t outputStatus[4];
      uint8_t inputPort[4];
      uint8_t outputPort[4];
      uint8_t filler1[7];
      uint8_t mac[6];
      uint32_t bindip;
      uint8_t bindIndex;
      uint8_t status2;
      uint8_t filler2[26];
    } pollReply;
    struct {
      uint8_t seq;
      uint8_t physical;
      uint16_t universe;
      uint16_t length;
      uint8_t data[512];
    } data;
  };
} inPkt;

struct ArtnetPacket outPkt = {
  {
    .ident={'A', 'r', 't', '-', 'n', 'e', 't', 0},
    .opcode=0x5100,
    .protocolVersion=0x0e00
  }
};

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

  if (anyChanged) {
    configSave();
    fillPollReply();
  }

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
  fillPollReply();
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
  if (anyChanged) {
    configSave();
    fillPollReply();
  }
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

void IotsaDMXMod::setHandler(uint8_t *_buffer, size_t _count, IotsaDMXHandler *_dmxHandler) {
  buffer = _buffer;
  count = _count;
  dmxHandler = _dmxHandler;
  fillPollReply();
}

void IotsaDMXMod::fillPollReply() {
  uint32_t ip = WiFi.localIP();
  if (ip == 0) {
  	ip = WiFi.softAPIP();
  }
  outPkt.pollReply.ipaddr = ip;
  outPkt.pollReply.port=ARTNET_PORT;
  outPkt.pollReply.version=1;
  outPkt.pollReply.dmxPort = portAddress & 0x7ff0;
  outPkt.pollReply.oem=0;
  outPkt.pollReply.ubea=0;
  outPkt.pollReply.status=0x10;
  outPkt.pollReply.esta=0xf07f;
  strncpy(outPkt.pollReply.shortName, shortName.c_str(), sizeof(outPkt.pollReply.shortName));
  strncpy(outPkt.pollReply.longName, longName.c_str(), sizeof(outPkt.pollReply.longName));
  strncpy(outPkt.pollReply.report, "#0001 [0000] All is well", sizeof(outPkt.pollReply.report));
  outPkt.pollReply.numPorts = 1;
  memset(outPkt.pollReply.portType, 0, sizeof(outPkt.pollReply.portType));
  outPkt.pollReply.portType[0] = 0x80;
  memset(outPkt.pollReply.inputStatus, 0, sizeof(outPkt.pollReply.inputStatus));
  memset(outPkt.pollReply.outputStatus, 0, sizeof(outPkt.pollReply.outputStatus));
  memset(outPkt.pollReply.inputPort, 0, sizeof(outPkt.pollReply.inputPort));
  memset(outPkt.pollReply.outputPort, 0, sizeof(outPkt.pollReply.outputPort));
  outPkt.pollReply.outputPort[0] = portAddress & 0xf;
  memset(outPkt.pollReply.mac, 0, sizeof(outPkt.pollReply.mac));
  outPkt.pollReply.bindip = ip;
  outPkt.pollReply.bindIndex=0;
  outPkt.pollReply.status2=0x08;

}

void IotsaDMXMod::loop() {
  size_t packetSize = udp.parsePacket();
  if (packetSize > sizeof(struct ArtnetHeader)) {
    if (packetSize > sizeof(struct ArtnetPacket)) {
      IFDEBUG IotsaSerial.println("Ignoring long packet");
      return;
    } 
    if (udp.read((char *)&inPkt, sizeof(inPkt)) != (int)packetSize) {
      IFDEBUG IotsaSerial.println("Ignoring incomplete packet");
      return;
    }
    if (strcmp(inPkt.header.ident, "Art-Net") != 0 || ntohs(inPkt.header.protocolVersion) != 14) {
      IFDEBUG IotsaSerial.print("Ignoring unknown packet, hdr=");
      IFDEBUG IotsaSerial.print(inPkt.header.ident);
      IFDEBUG IotsaSerial.print("version=");
      IFDEBUG IotsaSerial.println(inPkt.header.protocolVersion);
      return;
    }
    uint16_t opcode = inPkt.header.opcode;
    if (opcode == 0x5000) {
      IFDEBUG IotsaSerial.println("Data packet");
    } else if (opcode == 0x2000) {
      IFDEBUG IotsaSerial.println("Poll packet");
      udp.beginPacket(udp.remoteIP(), udp.remotePort());
      udp.write((uint8_t *)&outPkt, sizeof(outPkt));
      udp.endPacket();
    } else if (opcode == 0x2100) {
      IFDEBUG IotsaSerial.println("PollReply packet");
    } else {
      IFDEBUG IotsaSerial.print("Ignoring packet opcode=");
      IFDEBUG IotsaSerial.println(opcode, HEX);
    }
  }
}
