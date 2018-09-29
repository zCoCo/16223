#ifndef HAL_H
#define HAL_H
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// BODY VARIABLES:
int currentEyePercent = 0;

// EMOTION PRIMITIVES:
// Chuckles slightly by inverting the eyes three times and moving eye stalks up and down.
void chuckle();
// Opens/Close the Eyes by Drawing them at the Given Percent Closed where 0 is fully open and 1 is fully closed
void eyeLevel(int);
// Moves the Eye Lid Smoothly to the Given Eye Level.
void moveEyeLevelTo(int);
// Blinks Both Eyes by Lowering and Raising the Eye Level (Lids) Quickly. Returns Eye Lids to their initial state.
void blink();
// Blinks Both Eyes by Lowering and Raising the Eye Level (Lids) taking the Given Time to Complete. Returns Eye Lids to their initial state.
void blink(int);

// HELPER FUNCTIONS:
// Performs a Basic Blink/Squint by Inverting the Screen then Uninverting Shortly Later:
void invertBlink();

void initHAL(){
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.invertDisplay(true);
  display.display();
  display.clearDisplay();   // clears the screen and buffer
} // #initHAL

// Blinks Both Eyes by Lowering and Raising the Eye Level (Lids) Quickly. Returns Eye Lids to their initial state.
void blink(){ blink(0); }
// Blinks Both Eyes by Lowering and Raising the Eye Level (Lids) taking the Given Time to Complete. Returns Eye Lids to their initial state.
void blink(int t){
  static const char step = 5;
  int initState = currentEyePercent;
  int waitTime = step * t / 200;

  int i;
  for(i=initState; i<=100; i+=step){ // Close Eyes First
    eyeLids(i);
    delay(waitTime);
  }
  for(i=100; i>=0; i-=step){ // Then Open
    eyeLids(i);
    delay(waitTime);
  }
  for(i=0; i<=initState; i+=step){ // Return to Initial State
    eyeLids(i);
    delay(waitTime);
  }
} // #blink

void invertBlink(){
  display.invertDisplay(false);
  delay(250);
  display.invertDisplay(true);
} // #invertBlink

// Chuckles slightly by inverting the eyes three times and moving eye stalks up and down.
void chuckle(){
  invertBlink();
  invertBlink();
  invertBlink();
} // #chuckle

// Opens/Close the Eyes by Drawing them at the Given Percent Closed where 0 is fully open and 1 is fully closed.
#define LID_COLOR BLACK // Color of the eye-lid
void eyeLids(int percent){
  /* "Eye Levels" are lines drawn parallel to the line which runs diagonally
  across the display from lower-left to upper-right corner of the display. The
  value of the "lvl" is determined by the height at which the level must
  intersect the left edge of the display. */
  static const int w2h = display.width() / display.height(); // used to determine top edge intersection point
  static const int open_lvl = 0;
  static const int closed_lvl = 2*display.height();
  int curr_lvl = open_lvl + (closed_lvl-open_lvl)*currentEyePercent/100.0;
  int targ_lvl = open_lvl + (closed_lvl-open_lvl)*percent/100.0;

  // Determine whether lid-color lines need to be added or removed:
  unsigned char color = (currentEyePercent > percent) ? LID_COLOR : !LID_COLOR;
  // Change Eye-Level
  char dir = abs(targ_lvl-curr_lvl)/(targ_lvl-curr_lvl);
  while(curr_lvl != targ_lvl){
    curr_lvl += dir;
    display.drawLine(0,curr_lvl, w2h*curr_lvl,0, color);
  }

  display.display();
  currentEyePercent = percent;
} // #eyeLevel

// Moves the Eye Lid Smoothly to the Given Eye Level, Returns the Current Eye Level.
int moveEyeLidsTo(int targ_percent){
  /*static const char step = 5;
  int curr_lvl = eyeLevel(0);

  for(i=0; i<=100; i+=step){
    eyeLevel(i);
    delay(step*t/100);
  }*/
} // #moveEyeLevelTo

#endif // HAL_H
