#include "Tap2Bpm.h"

Tap2Bpm::Tap2Bpm(int t_r)
{
  this->tap_req = t_r;
  this->reset();
}

void Tap2Bpm::reset()
{
  this->counter = 0;
}

bool Tap2Bpm::tapNow(unsigned long _t)
{
  this->taptime[this->counter] = _t;
  this->counter++;
  return (this->getRemainingTaps() <= 0);
}

bool Tap2Bpm::checkBPM()
{
  unsigned int _b = 60000UL * 3 / (taptime[4] - taptime[1]);
  bool bpm_ok = (_b >= this->bpm_min && _b <= this->bpm_max);
  if (bpm_ok)
    this->bpm = _b;
  return bpm_ok;
}

unsigned int Tap2Bpm::getBPM()
{
  return this->bpm;
}

unsigned int Tap2Bpm::getRemainingTaps()
{
  return this->tap_req - this->counter;
}

unsigned int Tap2Bpm::getCount()
{
  return this->counter;
}