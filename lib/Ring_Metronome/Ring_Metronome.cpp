#include "Ring_Metronome.h"

Ring_Metronome::Ring_Metronome(Adafruit_NeoPixel &_p, unsigned int _pixelcount, unsigned int _pixeloffset)
{
    this->p = _p;
    this->totalpixels = _pixelcount;
    this->pixoff = _pixeloffset;
};

void Ring_Metronome::setup(unsigned int _beats)
{
    this->beats = _beats;
    this->tick_interval = totalpixels / this->beats; // 4 for quarter note, 2 for eigth note
};

unsigned int Ring_Metronome::index2pixel(unsigned int _i)
{
    return (_i + this->pixoff) % this->totalpixels;
};

void Ring_Metronome::setTicksRGB()
{
    this->p.setPixelColor(index2pixel(0), this->rgb_tick_1st);
    for (unsigned int i = this->tick_interval; i < this->totalpixels; i += this->tick_interval)
    {
        this->p.setPixelColor(index2pixel(i), this->rgb_tick_rest);
    }
};

void Ring_Metronome::update(bool _running, float _progress, unsigned int _flash)
{
    int32_t rgb = 0x00000000;
    // flashing on accent
    if (_flash == 2)
        rgb = this->rgb_flash_strong;
    else if (_flash == 1)
        rgb = this->rgb_flash_weak;
    this->p.fill(rgb);
    setTicksRGB();
    this->p.setPixelColor(index2pixel(round(this->totalpixels * _progress)), this->rgb_runner);
};

void Ring_Metronome::setColor(int32_t _r, int32_t _tk1, int32_t _tkr, int32_t _fl1, int32_t _flr)
{
    this->rgb_runner = _r,
    this->rgb_tick_1st = _tk1,
    this->rgb_tick_rest = _tkr;
    this->rgb_flash_strong = _fl1;
    this->rgb_flash_weak = _flr;
};