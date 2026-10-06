//
// A NeoPixel-strip server for lighting use. Individual pixel R/G/B (and W)
// values can be set through a web UI, through REST calls (and/or CoAP, depending
// on iotsa compile-time options), or driven over the network with Art-Net DMX.
// The web interface can be disabled by building iotsa with IOTSA_WITHOUT_WEB.
//

#include "iotsa.h"
#include "iotsaDMX.h"
#include "iotsaPixelstrip.h"

IotsaApplication application("Iotsa LED Server");

IotsaDMXMod dmxMod(application);
IotsaPixelstripMod pixelstripMod(application);

// Standard setup() method, hands off most work to the application framework
void setup(void){
  pixelstripMod.setDMX(&dmxMod);
  application.setup();
  application.lateSetup();
}

// Standard loop() routine, hands off most work to the application framework
void loop(void){
  application.loop();
}
