#ifndef _HAL_H
#define _HAL_H
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

/** Basic Motion Parameters: **/
const float GEAR_RATIO = 43.0 / 11.0; // Output to Input Gear Ratio
const float MOT_STEPS_PER_REV = 4075.7728 * GEAR_RATIO;

/** Series Elastic Parameters: **/
// Radial Position of the Mounting Point of the Rubber Bands on the Inner Disk [m]:
const float RP_INNER = 7.46e-3;
// Unloaded Length of Rubber Bands (when mounted in actuator):
#define L0 15.5e-3
// Amount of Stretching Required for Rubber Bands to Reach their Unloaded
// Position (L0) from their Relaxed Length:
#define d0 8e-3

void initHAL(){

} // #initHAL

#endif //_HAL_H
