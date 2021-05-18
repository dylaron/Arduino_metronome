/*
Visual-nome, A visual metronome with adafruit NeoPixel light ring. By DyLaron (Yuliang Deng)
*/
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <JC_Button.h>

#include "Beat_Gen.h"
#include "Defines.h"

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDRINGPIN);
Button myButton(BUTTONPIN, 80);

const bool init_start = AUTOSTART;
unsigned int bpm = INITBPM;
unsigned int beats_p_bar = BEATS;
unsigned int steps_p_beat = SUBBEAT;
bool upbeat = UPBEAT;
unsigned int steps_offset = steps_p_beat / 2 * upbeat;
char m = 'S', m_trapexit = 'S'; // R - running. S - standby. T - tapping
unsigned int tap_count = 0;
unsigned long taptime[6];

Beat_gen myBeat;
//----------------------------------------------------------
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
    pixels.setPixelColor((totalpixels - 1 - i + PIXELOFFSET) % 16, pixels.Color(rr, gg, bb));
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
}
//----------------------------------------------------------
void setup()
{
  myButton.begin();
  pixels.begin();
  pixels.setBrightness(45); // 1/3 brightness
  myBeat.setBeats(bpm, beats_p_bar, steps_p_beat);
  Serial.begin(9600);

  if (init_start)
  {
    myBeat.start(millis());
    m = 'R';
    Serial.print("Started (Auto-run)\n");
  }
}
//----------------------------------------------------------
void tap_prepare()
{
  pixels.fill(0x00008000, 0);
  pixels.show();
  tap_count = 0;
}
//----------------------------------------------------------
void loop()
{
  // refresh botton status
  myButton.read();
  bool buttonDrop = myButton.wasPressed();
  bool buttonRelease = myButton.wasReleased();
  bool longpress = myButton.pressedFor(1600);
  // Serial.print(m);
  switch (m)
  {
  case 'S':
    if (longpress) // Going to tap mode, while stopping at Y to catch the button release
    {
      tap_prepare();
      m = 'Z';
      m_trapexit = 'T';
    }
    else if (buttonRelease)
    {
      myBeat.start(myButton.lastChange());
      Serial.println("Started");
      m = 'R';
    }
    break;

  case 'R':
    // check if it's time on the beat
    if (myBeat.checktime())
    {
      unsigned int onbeat = (myBeat.current_step() == steps_offset) + ((myBeat.current_step() + steps_offset) % steps_p_beat == 0);
      led_ring_show(myBeat.running(), myBeat.progress_pct(), onbeat, beats_p_bar);
      if (onbeat == 2)
        tone(TONEPIN, 261, 60);
      else if (onbeat == 1)
        tone(TONEPIN, 196, 30);
    }
    if (buttonDrop)
    {
      myBeat.stop();
      m = 'Z';
      m_trapexit = 'S';
      Serial.println("Stopped");
    }
    break;

  case 'T':
    if (buttonRelease)
    {
      taptime[tap_count] = myButton.lastChange();
      tap_count++;
      pixels.fill(0x000000, (15 + PIXELOFFSET - tap_count * 3) % 16, 3); // turn off the leds as tapping proceeds
      pixels.show();
      if (tap_count >= 5)
      {
        unsigned long tap_interval = (taptime[4] - taptime[1]) / 3;
        unsigned int new_bpm = 60000 / tap_interval;
        Serial.println("New BPM:");
        Serial.println(new_bpm);
        if (new_bpm > 30 && new_bpm <= 240)
        {
          Serial.println("New BPM Valid!");
          bpm = new_bpm;
          myBeat.setBeats(bpm, beats_p_bar, steps_p_beat);
          myBeat.start(myButton.lastChange());
          Serial.println("Started (tap the beat)");
          m = 'R';
        }
        else
        {
          Serial.println("New BPM out of range!");
          tap_prepare();
        }
      }
    }
    if (longpress)
    {
      pixels.fill(0x000000);
      pixels.show();
      m = 'Z';
      m_trapexit = 'S';
    }
    break;

  case 'Z': //trap to catch a release action
    if (buttonRelease)
      m = m_trapexit;
    break;

  default:
    break;
  }
  delay(20);
}