#ifndef HAL_H
#define HAL_H
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>
#include <HCSR04.h>

// Ultrasound Sensing Pins:
#define P_ECHO 11
#define P_TRIG 12
UltraSonicDistanceSensor sonar(P_TRIG, P_ECHO);

// Servo Motor Pins:
#define P_LEFT_STALK 3
#define P_RIGHT_STALK 10
#define P_LEFT_HAND 6
#define P_RIGHT_HAND 9
Servo S_LEFT_STALK, S_RIGHT_STALK, S_LEFT_HAND, S_RIGHT_HAND;

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// BODY VARIABLES:
int currentEyePercent = 100;

// EMOTION PRIMITIVES:
// Chuckles slightly by inverting the eyes three times and moving eye stalks up and down.
void chuckle();
// Opens/Close the Eyes by Drawing them at the Given Percent Closed where 0 is fully open and 1 is fully closed
void eyeLids(int);
// Moves the Eye Lid Smoothly and Quickly to the Given Eye Level.
void moveEyeLidsTo(int);
// Blinks Both Eyes by Lowering and Raising the Eye Level (Lids) Quickly. Returns Eye Lids to their initial state.
void blink();
// Blinks Both Eyes by Lowering and Raising the Eye Level (Lids) taking the Given Time to Complete. Returns Eye Lids to their initial state.
void blink(int);

// MOTION PRIMITIVES:
// Moves stalks to %percent% of the way from the bottom of their swing where 0% is their lowest position and 100% is their highest
void moveStalks(int);
// Moves left stalk to %percent% of the way from the bottom of its swing where 0% is its lowest position and 100% is its highest
void moveStalkLeft(int);
// Moves right stalk to %percent% of the way from the bottom of its swing where 0% is its lowest position and 100% is its highest
void moveStalkRight(int);
// Moves hands to %percent% of the way from the bottom of their swing where 0% is their lowest position and 100% is their highest
void moveHands(int percent);
// Moves left hand to %percent% of the way from the bottom of its swing where 0% is its lowest position and 100% is its highest
void moveHandLeft(int percent);
// Moves right hand to %percent% of the way from the bottom of its swing where 0% is its lowest position and 100% is its highest
void moveHandRight(int percent);

// SENSING PRIMITIVES:
// Returns the distance to the nearest object in front of the robot based on ultrasound.
float dist();
// Returns Whether the Robot is Currently Being Touched on its Hands.
bool touched();

// HELPER FUNCTIONS:
// Performs a Basic Blink/Squint by Inverting the Screen then Uninverting Shortly Later:
void invertBlink();
// Commands the Given Servo to the Given Percent of its Range from minAng to maxAng
void commandServo(Servo, int, int, int);


void initHAL(){
  S_LEFT_STALK.attach(P_LEFT_STALK);
  S_RIGHT_STALK.attach(P_RIGHT_STALK);
  S_LEFT_HAND.attach(P_LEFT_HAND);
  S_RIGHT_HAND.attach(P_RIGHT_HAND);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.invertDisplay(false);
  display.clearDisplay();   // clears the screen and buffer
  display.drawRect(0,0,display.width(),display.height(),BLACK);
  display.display();
} // #initHAL

// Blinks Both Eyes by Lowering and Raising the Eye Level (Lids) Quickly. Returns Eye Lids to their initial state.
void blink(){ blink(0); }
// Blinks Both Eyes by Lowering and Raising the Eye Level (Lids) taking the Given Time to Complete. Returns Eye Lids to their initial state.
void blink(int t){
  static const char step = 5;
  int initState = currentEyePercent;
  int waitTime = step * t / 200;

  int i;
  for(i=initState; i<100; i+=step){ // Close Eyes First
    eyeLids(i);
    delay(waitTime);
  }
  for(i=100; i>0; i-=step){ // Then Open
    eyeLids(i);
    delay(waitTime);
  }
  for(i=0; i<=initState; i+=step){ // Return to Initial State
    eyeLids(i);
    delay(waitTime);
  }
} // #blink

void invertBlink(){
  display.invertDisplay(true);
  delay(250);
  display.invertDisplay(false);
} // #invertBlink

// Chuckles slightly by inverting the eyes three times and moving eye stalks up and down.
void chuckle(){
  invertBlink();
  moveStalks(80);
  delay(250);
  invertBlink();
  moveStalks(20);
  delay(250);
  invertBlink();
  moveStalks(100);
} // #chuckle

// Opens/Close the Eyes by Drawing them at the Given Percent Closed where 0 is fully open and 1 is fully closed.
#define LID_COLOR WHITE // Color of the eye-lid
void eyeLids(int percent){
  /* "Eye Levels" are lines drawn parallel to the line which runs diagonally
  across the display from lower-left to upper-right corner of the display. The
  value of the "lvl" is determined by the height at which the level must
  intersect the left edge of the display. */
  static const int w2h = display.width() / display.height(); // used to determine top edge intersection point
  #ifdef ROTATE_DISPLAY_180
    static const int open_lvl = 2*display.height();
    static const int closed_lvl = 0;
  #else
    static const int open_lvl = 0;
    static const int closed_lvl = 2*display.height();
  #endif
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

  display.display(); // this is slow, call infrequently
  currentEyePercent = percent;
} // #eyeLids

// Moves the Eye Lid Smoothly and Quickly to the Given Eye Level.
void moveEyeLidsTo(int targ_percent){
  // Change Eye-Level
  static const int step = 10;
  char dir = abs(targ_percent-currentEyePercent)/(targ_percent-currentEyePercent);
  int curr_target = currentEyePercent;
  while(currentEyePercent != targ_percent){
    curr_target += dir*step;
    eyeLids(curr_target);
  }
} // #moveEyeLidsTo

// Commands the Given Servo to the Given Percent of its Range from minAng to maxAng
void commandServo(Servo S, int minAng, int maxAng, int percent){
  S.write(minAng + (maxAng-minAng)*percent);
} // #commandServo
// Moves left stalk to %percent% of the way from the bottom of its swing where 0% is its lowest position and 100% is its highest
void moveStalkLeft(int percent){ commandServo(S_LEFT_STALK, 0, 70, percent); }
// Moves right stalk to %percent% of the way from the bottom of its swing where 0% is its lowest position and 100% is its highest
void moveStalkRight(int percent){ commandServo(S_RIGHT_STALK, 180, 155, percent); }
// Moves stalks to %percent% of the way from the bottom of their swing where 0% is their lowest position and 100% is their highest
void moveStalks(int percent){ moveStalkLeft(percent); moveStalkRight(percent); }

// Moves left hand to %percent% of the way from the bottom of its swing where 0% is its lowest position and 100% is its highest
void moveHandLeft(int percent){ commandServo(S_LEFT_HAND, 0, 80, percent); }
// Moves right hand to %percent% of the way from the bottom of its swing where 0% is its lowest position and 100% is its highest
void moveHandRight(int percent){ commandServo(S_RIGHT_HAND, 180, 100, percent); }
// Moves hands to %percent% of the way from the bottom of their swing where 0% is their lowest position and 100% is their highest
void moveHands(int percent){ moveHandLeft(percent); moveHandRight(percent); }

float dist(){
  return sonar.measureDistanceCm();
} // #dist
#endif // HAL_H
