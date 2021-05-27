/*
  A pulse generator. The purpose is to return a true on 'check()' every certain period.
  2021 Copyright (c)
  Author: 'DyLaron' Aaron Yuliang Deng
  Date: MAY-26-2021
*/

#ifndef PULSER_H
#define PULSER_H

#include <Arduino.h>

class Pulser
{
private:
    unsigned int period;
    unsigned long next_time;
    bool running;

public:
    Pulser();
    void setPeriod(unsigned int _p);
    void setFreq(unsigned int _f);
    void start(unsigned long atwhen);
    void reset();
    bool check();
    bool getRunning();
};

#endif