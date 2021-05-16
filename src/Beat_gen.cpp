#include "Beat_Gen.h"

Beat_gen::Beat_gen()
{
  this->_running = false;
}

void Beat_gen::setBeats(unsigned int bpm, unsigned int beats, unsigned int sub_div)
{
  this->step_count = beats * sub_div;
  this->time_step = round(60000UL / bpm / sub_div);
}

void Beat_gen::start(unsigned long atwhen)
{
  c_s = 0;
  this->_running = true;
  this->next_time = atwhen;
  _firsttick = true;
}

void Beat_gen::stop()
{
  this->_running = false;
}

bool Beat_gen::checktime()
{
  bool res = false;
  if (this->_firsttick)
  {
    res = true;
    this->_firsttick = false;
  }
  else if (millis() >= this->next_time)
  {
    res = true;
    this->next_time += this->time_step;
    this->c_s = (this->c_s + 1) % this->step_count;
  }
  return (res && _running);
}

unsigned int Beat_gen::current_step()
{
  return this->c_s;
}

bool Beat_gen::running()
{
  return this->_running;
}

float Beat_gen::progress_pct()
{
  return (float)c_s / step_count;
}