// IO pin assignments
#define BUTTONPIN 2 // a N.O. pushbutton between this and GND. Int Pull-up resistor to be used. Start/Stop
#define LEDRINGPIN 4 // input pin of the LED ring
#define TONEPIN 6 // a piezo buzzer, between this and GND.

#define NUMPIXELS 16
// Parameters
#define AUTOSTART true // start the metronome automatically upon power up and reset
#define INITBPM 96 // Default BPM to start with
#define BEATS 4 // Beats per bar (measure, cycle)
#define SUBBEAT 12 // Divide per beat into sub steps
#define UPBEAT false // True - Accent on the off/upbeat, False - on/downbeat
