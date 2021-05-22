#ifndef TAP2BPM_H
#define TAP2BPM_H

#include <Arduino.h>

class Tap2Bpm
{
private:
  unsigned int tap_req, counter, bpm, bpm_min = 40, bpm_max = 240;
  unsigned long taptime[10];
  bool ready;
  /* data */
public:
  Tap2Bpm(int t_r);
  void reset();
  bool tapNow(unsigned long _t); // return True if done
  bool checkBPM();
  unsigned int getBPM();
  unsigned int getRemainingTaps();
  unsigned int getCount();
};

#endif