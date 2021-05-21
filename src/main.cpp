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
class Ring_Metronome
{
private:
  unsigned int totalpixels;
  unsigned int beats, accent;
  Adafruit_NeoPixel p;
  int32_t rgb_runner = 0x00000080,
          rgb_tick_1st = 0x00802020,
          rgb_tick_rest = 0x00402020,
          rgb_flash_strong = 0x00808080,
          rgb_flash_weak = 0x00202020;

public:
  Ring_Metronome(Adafruit_NeoPixel &_p, unsigned int _pixelcount);
  void setup(unsigned int _beats);
  void show(bool _running, float _progress, unsigned int _flash);
};

Ring_Metronome::Ring_Metronome(Adafruit_NeoPixel &_p, unsigned int _pixelcount)
{
  this->totalpixels = _pixelcount;
  this->p = _p;
};

void Ring_Metronome::setup(unsigned int _beats)
{
  this->beats = _beats;
};

void Ring_Metronome::show(bool _running, float _progress, unsigned int _flash)
{
  int32_t rgb;
  const unsigned int tick_interval = totalpixels / this->beats; // 4 for quarter note, 2 for eigth note
  for (unsigned int i = 0; i < this->totalpixels; i++)
  {
    rgb = 0x00000000;
    // fixed ticks
    if (i == 0)
    {
      rgb = this->rgb_tick_1st;
    }
    else if (i % tick_interval == 0)
    {
      rgb = this->rgb_tick_rest;
    }

    if (_running)
    {
      // running dot
      if (i == round(this->totalpixels * _progress))
      {
        rgb = this->rgb_runner;
      }
      else // flashing on accent
      {
        if (_flash == 2)
          rgb = this->rgb_flash_strong;
        else if (_flash == 1)
          rgb = this->rgb_flash_weak;
      }
    }
    this->p.setPixelColor((this->totalpixels - 1 - i + PIXELOFFSET) % 16, rgb);
  }
  this->p.show(); // This sends the updated pixel color to the hardware.
};

Ring_Metronome myRing(pixels, NUMPIXELS);

//----------------------------------------------------------
void setup()
{
  myButton.begin();
  pixels.begin();
  pixels.setBrightness(45); // 1/3 brightness
  myBeat.setBeats(bpm, beats_p_bar, steps_p_beat);
  myRing.setup(beats_p_bar);
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
      myRing.show(myBeat.running(), myBeat.progress_pct(), onbeat);
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
      pixels.clear();
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