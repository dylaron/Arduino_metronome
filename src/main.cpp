/*
Visual-nome, A visual metronome with adafruit NeoPixel light ring. By DyLaron (Yuliang Deng)
*/
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <JC_Button.h>

#include "Beat_Gen.h"
#include "Defines.h"

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDRINGPIN);
ToggleButton myToggle(BUTTONPIN, AUTOSTART, 50);

const bool init_start = AUTOSTART;
unsigned int bpm = INITBPM;
unsigned int beats_p_bar = BEATS;
unsigned int steps_p_beat = SUBBEAT;

void led_ring_show(bool _running, float _progress, unsigned int _accent, unsigned int _beats)
{
  unsigned int rr, gg, bb;
  const unsigned int totalpixels = NUMPIXELS;
  const unsigned int r_runner = 0,
                     g_runner = 0,
                     b_runner = 128; // color of the runner
  const unsigned int r_1sttick = 96,
                     g_1sttick = 12,
                     b_1sttick = 12; // color of the 1st tick
  const unsigned int r_tick = 32,
                     g_tick = 32,
                     b_tick = 32;                          // color of the rest ticks
  const unsigned int tick_interval = totalpixels / _beats; // 4 for quarter note, 2 for eigth note
  const unsigned int flash_intensity = 24;
  for (unsigned int i = 0; i < totalpixels; i++)
  {
    // fixed ticks
    bool _1sttick = (i == 0);
    bool _ontick = ((i % tick_interval == 0) && !_1sttick);
    rr = r_1sttick * _1sttick + r_tick * _ontick;
    gg = g_1sttick * _1sttick + g_tick * _ontick;
    bb = b_1sttick * _1sttick + b_tick * _ontick;
    if (_running)
    {
      // running dot
      if (i == round(totalpixels * _progress))
      {
        rr = r_runner;
        gg = g_runner;
        bb = b_runner;
      }
      else // flashing on accent
      {
        unsigned int f = flash_intensity * _accent;
        rr += f;
        gg += f;
        bb += f;
      }
    }
    pixels.setPixelColor(totalpixels - 1 - i, pixels.Color(rr, gg, bb));
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
}

Beat_gen myBeat;

void setup()
{
  myToggle.begin();
  pixels.begin();
  pixels.setBrightness(45); // 1/3 brightness
  myBeat.setBeats(bpm, beats_p_bar, steps_p_beat);
  // Serial.begin(9600);

  if (init_start)
    myBeat.start(millis());
}
void loop()
{
  // refresh botton status
  myToggle.read();
  // execute button action
  if (myToggle.changed())
  {
    if (myToggle.toggleState())
      myBeat.start(myToggle.lastChange());
    else
      myBeat.stop();
  }
  // check if it's time on the beat
  bool ontime = myBeat.checktime();
  if (ontime)
  {
    unsigned int onbeat = (myBeat.current_step() == 0) + (myBeat.current_step() % steps_p_beat == 0);
    led_ring_show(myBeat.running(), myBeat.progress_pct(), onbeat, beats_p_bar);
    if (onbeat == 2)
      tone(TONEPIN, 261, 60);
    else if (onbeat == 1)
      tone(TONEPIN, 196, 30);
  }
}