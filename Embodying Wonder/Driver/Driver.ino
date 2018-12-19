/* Driving Script for Series Elastic Actuator controlling a mirror with a handle.
This involves a basic autonomous behaviour of bouncing back and forth between
-180 and 180 degrees but following the user's motion if the actuator detects
that its handle being grabbed. Once the user lets go, the actuator will stay at
the desired position for 1 second. */
// NB: CCW is +ve

#include "Arduino.h"
#include "HAL.h"
#include "Sensing.h"
#include "Motion.h"
#include "Schedule.h"
//#include "Comm.h"

#define sgn(x) ( (x==0) ? 0 : abs(x) / (x) )

Schedule* sch = new Schedule();

// Maximum Angular Difference between Input and Output before Actuator Enters Follower Mode:
#define DIFF_THRESH 12
bool is_following = false; // Whether Currently in Follower Mode
bool holding_position = false; // Whether Currently Holding the Position Set by the User
unsigned long let_go_time = 0; // Time when the user last let go

void setup(){
  Serial.begin(9600);
  initHAL();
  // initComm(); // -TODO: Implement I2C Communications for Sound Sync.

  schedule();
  moveTo(180); // Kick off the Autonomous Motion
} // #setup

void schedule(){
  /** Perform Basic Life-Line Tasks: **/
  sch->ALWAYS->DO(updateSensors);
  sch->ALWAYS->DO(updateMotion);

  /** Coordinate Responses: **/
  // Enter Follower Mode:
  sch->WHEN(Sensors.diff > DIFF_THRESH)->do_([](){
    is_following = true;
    move( sgn(Sensors.diff) * (abs(Sensors.diff) - DIFF_THRESH + 1) );
  });

  // Move to Rest at Position the User Set and Stay There for a Time:
  sch->WHEN(Sensors.diff < DIFF_THRESH)->do_([](){
    move( Sensors.diff );
    let_go_time = millis();
    holding_position = true;
  });

  // Exit Follower Mode and Resume Autonomous Operation after User has Let Go
  // for 1 Second:
  sch->WHEN(let_go_time - millis() > 1000 && holding_position)->do_([](){
    is_following = false;
    holding_position = false;
    moveTo(180);
  });

  sch->WHEN(idle() && !is_following)->do_([](){
    moveTo(-getCommAng()); // Bounce Back and Forth
  });

  /** Give Status Updates: **/
  // Plot Load on Actuator:
  sch->EVERY(200)->do_([](){
    Serial.print(Sensors.diff);
    Serial.print(",");
    Serial.println(torque());
  });
} // #schedule

void loop(){
  sch->loop();
} // #loop
