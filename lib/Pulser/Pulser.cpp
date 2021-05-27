#include "Pulser.h"

Pulser::Pulser()
{
}

void Pulser::setPeriod(unsigned int _p)
{
  this->period = _p;
}

void Pulser::setFreq(unsigned int _f)
{
  this->period = 1000 / _f;
}

void Pulser::start(unsigned long atwhen = millis())
{
  this->running = true;
  this->next_time = atwhen;
}

void Pulser::reset()
{
  this->running = false;
}

bool Pulser::check()
{
  bool res = ((millis() >= this->next_time) && this->running);
  if (res)
    this->next_time += this->period;
  return res;
}

bool Pulser::getRunning()
{
  return this->running;
}