/* Basic Code Base for Verifying All Sensor Functionality of an EWM Actuator */

#include "HAL.h"
#include "Sensing.h"

void setup(){
  Serial.begin(9600);
  initHAL();
} // #setup

unsigned long last_update = -10000;
void loop(){
  if(millis() - last_update > 100){
//    Serial.print(outputAng()); Serial.print(", ");
//    Serial.print(inputAng()); Serial.print(", ");
    Serial.println(Sensors.diff);
//    Serial.print(getCommAng());
//    Serial.print(", ");
//    Serial.println(outputAng());
    last_update = millis(); // Call at e of printing to have event e-start spacing
  }
  updateSensors();
} // #loop
