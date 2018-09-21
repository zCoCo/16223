// Requirements: RTL? (switch to list - issue with fixed-size vector?)
#include "Schedule.h"

float dist;

void setup(){
  schedule->EVERY(10)->do_(update_sonar);
  schedule->WHEN(dist < 10)->do_(backup);
} // #setup

void loop(){
  schedule->loop();
} // #loop
