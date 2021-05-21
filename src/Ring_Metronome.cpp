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
};

void Ring_Metronome::show(bool _running, float _progress, unsigned int _flash)
{
    int32_t rgb;
    const unsigned int tick_interval = totalpixels / this->beats; // 4 for quarter note, 2 for eigth note
    for (unsigned int i = 0; i < this->totalpixels; i++)
    {
        rgb = 0x00000000;
        // fixed ticks
        if (i == 0)
            rgb = this->rgb_tick_1st;
        else if (i % tick_interval == 0)
            rgb = this->rgb_tick_rest;

        if (_running)
        {
            // running dot
            if (i == round(this->totalpixels * _progress))
                rgb = this->rgb_runner;
            else // flashing on accent
            {
                if (_flash == 2)
                    rgb = this->rgb_flash_strong;
                else if (_flash == 1)
                    rgb = this->rgb_flash_weak;
            }
        }
        this->p.setPixelColor((this->totalpixels - 1 - i + this->pixoff) % 16, rgb);
    }
    this->p.show(); // This sends the updated pixel color to the hardware.
};

void Ring_Metronome::setColor(int32_t _r, int32_t _tk1, int32_t _tkr, int32_t _fl1, int32_t _flr)
{
    this->rgb_runner = _r,
    this->rgb_tick_1st = _tk1,
    this->rgb_tick_rest = _tkr;
    this->rgb_flash_strong = _fl1;
    this->rgb_flash_weak = _flr;
};