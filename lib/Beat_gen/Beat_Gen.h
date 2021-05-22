#ifndef BEAT_GEN_H
#define BEAT_GEN_H

#include <Arduino.h>

class Beat_gen
{
private:
  unsigned long next_time;
  unsigned int time_step;
  unsigned int step_count;
  unsigned int c_s;
  bool _running;
  bool _firsttick;

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