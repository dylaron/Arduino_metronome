/*
Visual-nome, A visual metronome with adafruit NeoPixel light ring. By DyLaron (Yuliang Deng)
*/
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <JC_Button.h>
#include <Fsm.h> // arduino-fsm Library by Jon Black

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
bool upbeat = UPBEAT,
     long_pressed_fired = false,
     tapping_accepted = false,
     ignore_next_release = false;

unsigned int steps_offset = steps_p_beat / 2 * upbeat;

Beat_gen myBeat;
Ring_Metronome myRing(pixels, NUMPIXELS, PIXELOFFSET);
Tap2Bpm myTapper(5);

//----------------------------------------------------------

// Trigger Events for the State Machine --------------------
#define BUTTON_PRESSED_EVENT 0
#define BUTTON_RELEASED_EVENT 1
#define BUTTON_LONGPRESS_EVENT 2
#define TAPPING_SUCC_EVENT 3

//State Machine States
void on_standby_enter()
{
  myRing.setTicksRGB();
  pixels.show();
}

//----------------------------------------------------------
void on_active_enter()
{
  myBeat.start(myButton.lastChange());
  Serial.println("Started");
}
//----------------------------------------------------------
void on_active_state()
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
}
//----------------------------------------------------------
void on_active_exit()
{
  ignore_next_release = true;
  myBeat.stop();
  Serial.println("Stopped");
}
//----------------------------------------------------------
void on_tapping_enter()
{
  Serial.println("Tapping Ready");
  pixels.fill(0x00008000, 0);
  pixels.show();
  myTapper.reset();
  ignore_next_release = true;
}
//----------------------------------------------------------
void on_tapping_state()
{
  if (myButton.wasReleased())
    if (!ignore_next_release)
    {
      bool doneTapping = myTapper.tapNow(myButton.lastChange());
      pixels.fill(0x000000, (15 + PIXELOFFSET - myTapper.getCount() * 3) % 16, 3); // turn off the leds as tapping proceeds
      pixels.show();
      if (doneTapping)
      {
        if (myTapper.checkBPM())
        {
          bpm = myTapper.getBPM();
          Serial.println("New BPM = ");
          Serial.println(bpm);
          myBeat.setBeats(bpm, beats_p_bar, steps_p_beat);
          tapping_accepted = true;
        }
        else
        {
          Serial.println("New BPM out of range!");
          pixels.fill(0x00008000, 0);
          pixels.show();
          myTapper.reset();
        }
      }
      else
        ignore_next_release = false;
    }
}

//State Machine Declaration
State state_standby(&on_standby_enter, NULL, NULL);
State state_active(&on_active_enter, &on_active_state, &on_active_exit);
State state_tapping(&on_tapping_enter, &on_tapping_state, NULL);
Fsm fsm_metronome(&state_standby);
//----------------------------------------------------------
void checkButtonsAndTrig(Button &_b, Fsm &_fsm)
{
  if (_b.wasPressed())
  {
    _fsm.trigger(BUTTON_PRESSED_EVENT);
    long_pressed_fired = false;
  }

  if (_b.pressedFor(1600)) // Going to tap mode
  {
    if (!long_pressed_fired)
    {
      _fsm.trigger(BUTTON_LONGPRESS_EVENT);
      long_pressed_fired = true;
      ignore_next_release = true;
    }
  }
  else if (_b.wasReleased())
  {
    if (!ignore_next_release)
      _fsm.trigger(BUTTON_RELEASED_EVENT);
    else
      ignore_next_release = false;
  }
}
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

  fsm_metronome.add_transition(&state_standby, &state_active, BUTTON_RELEASED_EVENT, NULL);
  fsm_metronome.add_transition(&state_standby, &state_tapping, BUTTON_LONGPRESS_EVENT, NULL);
  fsm_metronome.add_transition(&state_tapping, &state_standby, BUTTON_LONGPRESS_EVENT, NULL);
  fsm_metronome.add_transition(&state_active, &state_standby, BUTTON_PRESSED_EVENT, NULL);
  fsm_metronome.add_transition(&state_tapping, &state_active, TAPPING_SUCC_EVENT, NULL);
}
//----------------------------------------------------------
void loop()
{
  myButton.read(); // refresh botton status
  fsm_metronome.run_machine();
  checkButtonsAndTrig(myButton, fsm_metronome);
  if (tapping_accepted)
  {
    fsm_metronome.trigger(TAPPING_SUCC_EVENT);
    tapping_accepted = false;
  }
  delay(20);
}
