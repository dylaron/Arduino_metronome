/*
  A beat generator for a metronome
  2021 Copyright (c)
  Author: 'DyLaron' Aaron Yuliang Deng
  Date: MAY-26-2021
*/

#ifndef BEAT_GEN_H
#define BEAT_GEN_H

#include <Arduino.h>
#include "Pulser.h"

class Beat_gen
{
private:
  unsigned int step_count;
  unsigned int c_s;
  Pulser myPulse;

public:
  Beat_gen();
  void setBeats(unsigned int bpm, unsigned int beats, unsigned int sub_div);
  void start(unsigned long atwhen);
  void stop();
  bool checktime();
  bool running();
  unsigned int current_step();
  float progress_pct();
};

#endif