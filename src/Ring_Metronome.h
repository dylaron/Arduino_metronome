#ifndef RING_METRONOME_H
#define RING_METRONOME_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class Ring_Metronome
{
private:
  unsigned int totalpixels, pixoff;
  unsigned int beats, accent;
  Adafruit_NeoPixel p;
  int32_t rgb_runner = 0x00000080,
          rgb_tick_1st = 0x00A0A000,
          rgb_tick_rest = 0x00404010,
          rgb_flash_strong = 0x00808080,
          rgb_flash_weak = 0x00202020;

public:
  Ring_Metronome(Adafruit_NeoPixel &_p, unsigned int _pixelcount, unsigned int _pixeloffset);
  void setup(unsigned int _beats);
  void show(bool _running, float _progress, unsigned int _flash);
  void setColor(int32_t _r, int32_t _tk1, int32_t _tkr, int32_t _fl1, int32_t _flr);
};

#endif
