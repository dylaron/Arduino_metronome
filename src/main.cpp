/*
Visual-nome, A visual metronome with adafruit NeoPixel light ring. By DyLaron (Yuliang Deng)
*/
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <JC_Button.h>
#include <FlashStorage.h> // To store bpm
#include <Fsm.h>          // arduino-fsm Library by Jon Black
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RotaryEncoder.h>

#include "Defines.h"        // all configurable variables are there
#include "Beat_Gen.h"       // This class deals with timing
#include "Ring_Metronome.h" // This class deals with the visual rendering
#include "Tap2Bpm.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDRINGPIN);
RotaryEncoder encoder(ROTARY_PIN_A, ROTARY_PIN_B, RotaryEncoder::LatchMode::FOUR0);
Button myButton(BUTTONPIN);

const bool init_start = AUTOSTART;
unsigned int bpm = INITBPM;
unsigned int beats_p_bar = BEATS;
unsigned int steps_p_beat = SUBBEAT;
bool upbeat = UPBEAT,
     long_pressed_fired = false,
     tapping_accepted = false,
     ignore_next_release = false;

unsigned int steps_offset = steps_p_beat / 2 * upbeat;
int current_menu_item = 1;

Beat_gen myBeat;
Ring_Metronome myRing(pixels, NUMPIXELS, PIXELOFFSET);
Tap2Bpm myTapper(5);

// Reserve a portion of flash memory to store the recipe parameters
int flash_init_mark = 173;
FlashStorage(my_flash_init_code, int);
FlashStorage(my_flash_bpm, unsigned int);
//----------------------------------------------------------
void bpm_in_binary(Adafruit_NeoPixel &_p, unsigned int _b)
{
  _p.clear();
  for (int i = 0; i < 8; i++)
  {
    bool bit = (_b & (1 << i)) != 0;
    uint32_t col = 0x101000;
    if (bit)
      col = 0x00B0C0;
    _p.setPixelColor(4 + i, col);
  }
  _p.show();
}
//----------------------------------------------------------
void checkPosition()
{
  encoder.tick(); // just call tick() to check the state.
}
// Trigger Events for the State Machine --------------------
#define BUTTON_PRESSED_EVENT 0
#define BUTTON_RELEASED_EVENT 1
#define BUTTON_LONGPRESS_EVENT 2
#define TAPPING_SUCC_EVENT 3

//State Machine States
void on_standby_enter()
{
  // myRing.setTicksRGB();
  // pixels.show();
  bpm_in_binary(pixels, bpm);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.println(" TAP  START  +/-");
}
//----------------------------------------------------------
void on_standby_state()
{
  const int cur_location[3] = {0, 5, 12};
  lcd.setCursor(cur_location[current_menu_item], 0);
  lcd.print(">");
  int new_dir = (int)encoder.getDirection();
  if (new_dir != 0)
  {
    lcd.setCursor(cur_location[current_menu_item], 0);
    lcd.print(" ");
    current_menu_item += new_dir;
    current_menu_item = (current_menu_item + 6) % 3;
    Serial.println(new_dir);
  }
}
//----------------------------------------------------------
void on_active_enter()
{
  myBeat.start(myButton.lastChange());
  Serial.println("Started");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.println("Running");
  lcd.print(bpm);
  lcd.print("BPM");
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
          my_flash_bpm.write(bpm);
          my_flash_init_code.write(flash_init_mark);
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
State state_standby(&on_standby_enter, &on_standby_state, NULL);
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
  lcd.init();
  lcd.backlight();
  pixels.begin();
  pixels.setBrightness(45); // 1/3 brightness
  myBeat.setBeats(bpm, beats_p_bar, steps_p_beat);
  myRing.setup(beats_p_bar);
  myRing.setColor(RGBRUNNER, RGB1STTICK, RGBRESTTICK, RGBFLASHS, RGBFLASHW);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(ROTARY_PIN_A), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_PIN_B), checkPosition, CHANGE);

  fsm_metronome.add_transition(&state_standby, &state_active, BUTTON_RELEASED_EVENT, NULL);
  fsm_metronome.add_transition(&state_standby, &state_tapping, BUTTON_LONGPRESS_EVENT, NULL);
  fsm_metronome.add_transition(&state_tapping, &state_standby, BUTTON_LONGPRESS_EVENT, NULL);
  fsm_metronome.add_transition(&state_active, &state_standby, BUTTON_PRESSED_EVENT, NULL);
  fsm_metronome.add_transition(&state_tapping, &state_active, TAPPING_SUCC_EVENT, NULL);

  if (my_flash_init_code.read() == flash_init_mark)
  {
    bpm = my_flash_bpm.read();
  }
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

  static int pos = 0;
  encoder.tick();

  int newPos = encoder.getPosition();
  if (pos != newPos)
  {
    lcd.setCursor(0, 1);
    lcd.print(newPos);
    lcd.print(" ");
    pos = newPos;
  } // if

  //delay(5);
}
