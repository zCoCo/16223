#ifndef _HAL_H
#define _HAL_H
/** Hardware Abstraction Layer **/
// Uses ESP8266 12-F (AI-Thinker Variant)
// Program as Adafruit Feather HUZZAH
// Flash: 4M
// No Debug
// lwIP: v2 Lower Memory
// CPU: 80MHz
// Baud: 115200
// Erase: Sketch Only

#include <Encoder.h>
#define ENC_STEPS_PER_REV 80.0
Encoder EncO(13,12); // Output Encoder
Encoder EncI(10,9); // Input Encoder

#include <AccelStepper.h>
#define STP 1
#define DIR 3
#define EN 8
#define MS1 6
#define MS2 4
#define MS3 5
AccelStepper stepper(1, STP, DIR);

/** Basic Motion Parameters: **/
const float GEAR_RATIO = 43.0 / 11.0; // Output to Input Gear Ratio
const float MOT_STEPS_PER_REV = 4075.7728 * GEAR_RATIO; // Account for internal gearbox

/** Series Elastic Parameters: **/
// Radial Position of the Mounting Point of the Rubber Bands on the Inner Disk [m]:
const float RP_INNER = 7.46e-3;
// Unloaded Length of Rubber Bands (when mounted in actuator):
const float L0 = 15.5e-3;
// Amount of Stretching Required for Rubber Bands to Reach their Unloaded
// Position (L0) from their Relaxed Length:
#define d0 8e-3
// Number of Rubber Bands:
#define N_BANDS 4
// Average Effective Stiffness of Each Rubber Band [N/m]:
#define K_BAND 15


void initHAL(){
  // Initialize Motor Driver Pins and Setup for Full-Step Mode:
  pinMode(STP, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(EN, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);
  digitalWrite(MS1, 0);
  digitalWrite(MS2, 0);
  digitalWrite(MS3, 0);
  digitalWrite(EN, 0);

  // Setup Motor Control Parameters:
  stepper.setMaxSpeed(100);
  stepper.setAcceleration(1000);
} // #initHAL

#endif //_HAL_H
