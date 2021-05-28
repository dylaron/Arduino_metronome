// IO pin assignments
#define BUTTONPIN 3    // a N.O. pushbutton between this and GND. Int Pull-up resistor to be used. Start/Stop
#define LEDRINGPIN 4   // input pin of the LED ring
#define TONEPIN 9      // a piezo buzzer, between this and GND.
#define PIXELOFFSET 14 // offset for led pixel to get the 1st one at top.

#define NUMPIXELS 16
// Parameters
#define AUTOSTART false // start the metronome automatically upon power up and reset
#define INITBPM 96      // Default BPM to start with
#define BEATS 4         // Beats per bar (measure, cycle)
#define SUBBEAT 12      // Divide per beat into sub steps
#define UPBEAT false    // True - Accent on the off/upbeat, False - on/downbeat

#define RGBRUNNER 0x00000080   // color for the runner dot
#define RGB1STTICK 0x00C0A000  // color for the first tick
#define RGBRESTTICK 0x00404010 // color for the rest of ticks
#define RGBFLASHS 0x00606060   // color for the major flash /accent
#define RGBFLASHW 0x00202020   // color for the rest flash /accent