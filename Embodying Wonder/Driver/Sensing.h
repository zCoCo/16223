#ifndef _SENSING_H
#define _SENSING_H

#include "HAL.h"

struct SensorsType{
  // Useful Data:
  float input_ang = 0.0; // - Angle of Input Disk
  float output_ang = 0.0; //- Angle of Output Disk
  float diff = 0.0; //      - Angular Difference between Input and Output Disks [deg]

  // Helper Variables:
  float lag_sum = 0.0; // Sum of all measured values for diff
  unsigned long lag_count = 0; // Number of measured values for diff
} Sensors;

// Returns the Output Angle from the Encoder in Degrees
float outputAng(){
  return 360.0 * EncO.read() / ENC_STEPS_PER_REV;
} // #outputAng

// Returns the Input Angle from the Encoder in Degrees
float inputAng(){
  return -360.0 * EncI.read() / ENC_STEPS_PER_REV / GEAR_RATIO;
} // #outputAng

// Computes the Torque Loading the Actuator in N-m. This is an expensive
// calculation, only call on an as-needed basis:
float torque(){
  // Constant Geometric Helper Parameters to Speed Up Calculations:
  static const float L0_2 = sq(L0);
  static const float A = 2 * RP_INNER * (L0 + RP_INNER);
  static const float L0_d = d0 - L0;

  // Compute Torque (only valid for diff <= 180deg, bands will snap before this):
  const float th = Sensors.diff * M_PI / 180.0;
  const float cm = cos(th) - 1;
  return N_BANDS * RP_INNER * K_BAND * (sqrt(L0_2 - A*cm) + L0_d) * sin( th + atan(RP_INNER * sin(th) / (L0 - RP_INNER*cm)) );
} // #torque

// Update Sensor Metadata:
void updateSensors(){
  Sensors.input_ang = inputAng();
  Sensors.output_ang = outputAng();
  Sensors.lag_sum += Sensors.output_ang - Sensors.input_ang;
  Sensors.lag_count += 1;
  Sensors.diff = Sensors.output_ang - Sensors.input_ang - Sensors.lag_sum / Sensors.lag_count;
} // #updateSensors

#endif //_SENSING_H
