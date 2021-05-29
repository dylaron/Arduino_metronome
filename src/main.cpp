/*
Visual-nome, A visual metronome with adafruit NeoPixel light ring. By DyLaron (Yuliang Deng)
*/
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <JC_Button.h>

#include "Defines.h"        // all configurable variables are there
#include "Beat_Gen.h"       // This class deals with timing
#include "Ring_Metronome.h" // This class deals with the visual rendering
#include "Tap2Bpm.h"

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDRINGPIN);
Button myButton(BUTTONPIN, 80);

const bool init_start = AUTOSTART;
unsigned int bpm = INITBPM;
unsigned int beats_p_bar = BEATS;
unsigned int steps_p_beat = SUBBEAT;
bool upbeat = UPBEAT;
unsigned int steps_offset = steps_p_beat / 2 * upbeat;

Beat_gen myBeat;
Ring_Metronome myRing(pixels, NUMPIXELS, PIXELOFFSET);
Tap2Bpm myTapper(5);

//----------------------------------------------------------
class State
{
public:
  virtual ~State() {}
  virtual void on_enter(){};
  virtual State *on_state() = 0;
  virtual void on_exit(){};
};

class ActiveState : public State
{
public:
  void on_enter();
  State *on_state();
};

class StandbyState : public State
{
public:
  void on_enter();
  State *on_state();
};

class TappingState : public State
{
public:
  void on_enter();
  State *on_state();
};
//----------------------------------------------------------
static ActiveState active_state;
static StandbyState standby_state;
static TappingState tapping_state;
//----------------------------------------------------------
void ActiveState::on_enter()
{
  myBeat.start(myButton.lastChange());
}

State *ActiveState::on_state()
{
  // check if it's time on the beat
  if (myBeat.checktime())
  {
    unsigned int onbeat = (myBeat.current_step() == steps_offset) + ((myBeat.current_step() + steps_offset) % steps_p_beat == 0);
    myRing.update(myBeat.running(), myBeat.progress_pct(), onbeat);
    pixels.show();
    if (onbeat == 2)
      tone(TONEPIN, 261, 60);
    else if (onbeat == 1)
      tone(TONEPIN, 196, 30);
  }
  if (myButton.wasPressed())
  {
    myBeat.stop();
    while (myButton.isPressed())
    {
      myButton.read();
    }
    return &standby_state;
  }
  else
    return this;
}

void StandbyState::on_enter()
{
  myRing.setTicksRGB();
  pixels.show();
}

State *StandbyState::on_state()
{
  if (myButton.pressedFor(1600)) // Going to tap mode, while stopping at Y to catch the button release
  {
    pixels.fill(0x00008000, 0);
    pixels.show();
    while (myButton.isPressed())
    {
      myButton.read();
    };
    return &tapping_state;
  }
  else if (myButton.wasReleased())
  {
    //myBeat.start(myButton.lastChange());
    Serial.println("Started");
    return &active_state;
  }
  else
    return this;
}

void TappingState::on_enter()
{
  myTapper.reset();
}

State *TappingState::on_state()
{
  if (myButton.wasReleased())
  {
    bool doneTapping = myTapper.tapNow(myButton.lastChange());
    pixels.fill(0x000000, (15 + PIXELOFFSET - myTapper.getCount() * 3) % 16, 3); // turn off the leds as tapping proceeds
    pixels.show();
    if (doneTapping)
    {
      if (myTapper.checkBPM())
      {
        Serial.println("New BPM Valid!");
        bpm = myTapper.getBPM();
        Serial.println(bpm);
        myBeat.setBeats(bpm, beats_p_bar, steps_p_beat);
        myBeat.start(myButton.lastChange());
        Serial.println("Started (tap the beat)");
        return &active_state;
      }
      else
      {
        Serial.println("New BPM out of range!");
        pixels.fill(0x00008000, 0);
        pixels.show();
        myTapper.reset();
      }
    }
  }
  else if (myButton.pressedFor(1600))
  {
    pixels.clear();
    pixels.show();
    while (myButton.isPressed())
    {
      myButton.read();
    }
    return &standby_state;
  }
  return this;
}

//---------------------------------------------------------
bool isChanged = false;
State *nextState = &standby_state;
State *nowState = nextState;
//----------------------------------------------------------
void setup()
{
  myButton.begin();
  pixels.begin();
  pixels.setBrightness(45); // 1/3 brightness
  myBeat.setBeats(bpm, beats_p_bar, steps_p_beat);
  myRing.setup(beats_p_bar);
  myRing.setColor(RGBRUNNER, RGB1STTICK, RGBRESTTICK, RGBFLASHS, RGBFLASHW);
  Serial.begin(9600);

  if (init_start)
  {
    nowState = &active_state;
    myBeat.start(millis());
    Serial.print("Started (Auto-run)\n");
  }
}
//----------------------------------------------------------
void loop()
{
  // refresh botton status
  myButton.read();
  // if state changed last time, call enter
  if (isChanged)
    nowState->on_enter();
  // call run repeatedly
  nextState = nowState->on_state();
  // if state changed, record it;
  isChanged = (nextState != nowState);
  // reset lastState to current
  nowState = nextState;

  delay(20);
}