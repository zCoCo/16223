/* Basic Code Base for Verifying All Sensor Functionality of an EWM Actuator */

#include "HAL.h"
#include "Sensing.h"
#include "Schedule.h"

Schedule* sch = new Schedule();

void setup(){
  Serial.begin(9600);
  initHAL();
  schedule();
} // #setup

void schedule(){
  sch->EVERY(200)->do_([](){
    Serial.println(Sensors.diff);
  });

  sch->ALWAYS->DO(updateSensors);
}

void loop(){
  sch->loop();
} // #loop
