#include <AccelStepper.h>
#include <HCSR04.h>
#include "Schedule.h"

Schedule* sch = new Schedule();

// Ultrasound Sensing Pins:
#define P_ECHO 9
#define P_TRIG 10
UltraSonicDistanceSensor sonar(P_TRIG, P_ECHO);

#define BUZZ A0

void beep(){
  tone(BUZZ, 700, 100);
}

// BEHAVIOR PARAMETERS:
// Time before "bomb" goes off [ms]:
#define TIME_TO_DETONATION 20000
// Slowest (and steady state) tick duration:
const unsigned int max_tick_time = 1000;
// Fastest tick duration:
const unsigned int min_tick_time = 100;

// Distance of Observer at Which the the Trick Hands are Fully Unfurled:
const float trick_dist = 30;
// Distance of Observer at Which the the Trick Hands are Fully Hidden and the Clock Behaves Normally:
const float norm_dist = 200;

// STATE VARIABLES:
// Duration of each clock tick during steady state [ms]:
unsigned int tick_time;

// Clock Time at Which Behavior Begins [ms]:
unsigned long start_time;

// Seconds Hand:
#define STP1 3
#define DIR1 4
// Trick Hand:
#define STP2 6
#define DIR2 7
AccelStepper SecondsHand(1, STP1, DIR1);
AccelStepper TrickHand(1, STP2, DIR2);

// Returns Distance to the Observer in cm.
float dist(){
  return sonar.measureDistanceCm();
} // #dist

// Returns the Current Time Elapsed as a Fraction of the Total Time to Detonation:
float timeElapsed(){
  return (millis() - start_time) / TIME_TO_DETONATION;
} // #timeElapsed

// Behavior to be Performed During Each Tick:
void tick(){
  beep();
  SecondsHand.moveTo(360 * timeElapsed());
  SecondsHand.setSpeed(200);
  SecondsHand.runSpeedToPosition();
  if(timeElapsed() < 1.0){
    sch->IN(tick_time)->do_(tick);
  } else{
    tone(BUZZ, 300, 1000);
  }
} // #tick

void setup(){
  // Setup Hardware:
  SecondsHand.setMaxSpeed(500);
  TrickHand.setMaxSpeed(500);

  // Setup Behaviors:
  // Start Ticking Faster Once Half Time Has Elapsed
  sch->WHILE(timeElapsed() > 0.5)->do_([](){
    tick_time = max_tick_time - (max_tick_time-min_tick_time) * (timeElapsed() - 0.5)/0.5;
  });

  // Establish Deceptive Behavior of TrickHand:
  sch->WHILE(true)->do_([](){
    int trickState = 360 - 360 * (dist() - trick_dist) / (norm_dist - trick_dist);
    TrickHand.moveTo( SecondsHand.currentPosition() + constrain(trickState, 0, 360) );
    TrickHand.setSpeed(200);
    TrickHand.runSpeedToPosition();
  });

  // Start Ticking:
  sch->NOW->do_(tick);

  // Begin:
  tick_time = max_tick_time;
  start_time = millis();
} // #setup

void loop(){
  sch->loop();
} // #loop
