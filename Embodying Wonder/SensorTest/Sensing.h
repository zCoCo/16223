#ifndef _SENSING_H
#define _SENSING_H

#include "HAL.h"

struct SensorsType{
  float diff = 0.0;
  float lag_sum = 0.0;
  unsigned long lag_count = 0;
} Sensors;

// Returns the Output Angle from the Encoder in Degrees
float outputAng(){
  return 360.0 * EncO.read() / ENC_STEPS_PER_REV;
} // #outputAng

// Returns the Input Angle from the Encoder in Degrees
float inputAng(){
  return -360.0 * EncI.read() / ENC_STEPS_PER_REV / GEAR_RATIO;
} // #outputAng

// Update Sensor Metadata:
void updateSensors(){
  Sensors.lag_sum += outputAng() - inputAng();
  Sensors.lag_count += 1;
  Sensors.diff = outputAng() - inputAng() - Sensors.lag_sum / Sensors.lag_count;
} // #updateSensors

#endif //_SENSING_H
