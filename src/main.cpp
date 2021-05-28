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
char currentState = 'S', trapExitState = 'S'; // R - running. S - standby. T - tapping

Beat_gen myBeat;
Ring_Metronome myRing(pixels, NUMPIXELS, PIXELOFFSET);
Tap2Bpm myTapper(5);
//----------------------------------------------------------
void tap_prepare()
{
  pixels.fill(0x00008000, 0);
  pixels.show();
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

  myRing.setTicksRGB();
  pixels.show();

  Serial.begin(9600);

  if (init_start)
  {
    myBeat.start(millis());
    currentState = 'R';
    Serial.print("Started (Auto-run)\n");
  }
}
//----------------------------------------------------------
void loop()
{
  // refresh botton status
  myButton.read();
  bool buttonDrop = myButton.wasPressed();
  bool buttonRelease = myButton.wasReleased();
  bool longpress = myButton.pressedFor(1600);
  switch (currentState)
  {
  case 'S':
    if (longpress) // Going to tap mode, while stopping at Y to catch the button release
    {
      tap_prepare();
      currentState = 'Z';
      trapExitState = 'T';
    }
    else if (buttonRelease)
    {
      myBeat.start(myButton.lastChange());
      Serial.println("Started");
      currentState = 'R';
    }
    break;

  case 'R':
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
    if (buttonDrop)
    {
      myBeat.stop();
      currentState = 'Z';
      trapExitState = 'S';
      Serial.println("Stopped");
    }
    break;

  case 'T':
    if (buttonRelease)
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
          currentState = 'R';
        }
        else
        {
          Serial.println("New BPM out of range!");
          tap_prepare();
        }
        myTapper.reset();
      }
    }
    if (longpress)
    {
      pixels.clear();
      pixels.show();
      currentState = 'Z';
      trapExitState = 'S';
    }
    break;

  case 'Z': //trap to catch a release action
    if (buttonRelease)
      currentState = trapExitState;
    break;

  default:
    break;
  }
  delay(20);
}