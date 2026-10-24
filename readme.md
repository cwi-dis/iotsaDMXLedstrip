# DMX Ledstrip controller

![build-platformio](https://github.com/cwi-dis/iotsaDMXLedstrip/workflows/build-platformio/badge.svg)
![build-arduino](https://github.com/cwi-dis/iotsaDMXLedstrip/workflows/build-arduino/badge.svg)

This iotsa application controls led strips.

It allows control over individual pixel R, G, B
and possibly W values using Rest calls, or
using Art-Net DMX lighting commands.

## Not to be confused with lissabonLedstrip

The hardware is much the same as [lissabonLedstrip](https://github.com/cwi-dis/lissabon), but the use is completely different:

- **iotsaDMXLedstrip (this one) is for effects.** Each LED has a 3D-printed adapter that sends its light into a fibre strand. The strands hang down at the back or front of a stage, and DMX (or REST) commands drive effects: rainbow ripples moving across the stage, a giant VU meter beating to the music. The pixel data changes many times a second.
- **lissabonLedstrip is ambient lighting**, for example a strip on the ceiling. The LEDs work together to give "normal" light for everyday use.

That is why the two differ in what needs which right: here the pixel data is open to everyone on the network (as Art-Net is), setting up for a show (gamma, test mode, the DMX settings) needs the right `dmx`, and the wiring needs `config`.
