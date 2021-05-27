#include "Beat_Gen.h"
#include "Pulser.h"

Beat_gen::Beat_gen()
{
}

void Beat_gen::setBeats(unsigned int bpm, unsigned int beats, unsigned int sub_div)
{
  this->step_count = beats * sub_div;
  unsigned int _p = round(60000UL / bpm / sub_div);
  this->myPulse.setPeriod(_p);
}

void Beat_gen::start(unsigned long atwhen = millis())
{
  this->myPulse.start(atwhen);
  c_s = this->step_count - 1;
}

void Beat_gen::stop()
{
  this->myPulse.reset();
}

bool Beat_gen::checktime()
{
  bool res = this->myPulse.check();
  if (res)
  {
    this->c_s = (this->c_s + 1) % this->step_count;
  }
  return res;
}

unsigned int Beat_gen::current_step()
{
  return this->c_s;
}

bool Beat_gen::running()
{
  return this->myPulse.getRunning();
}

float Beat_gen::progress_pct()
{
  return (float)c_s / step_count;
}