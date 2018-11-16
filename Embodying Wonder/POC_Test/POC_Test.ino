/* Basic Code Base for Verifying All Core Functionality on a
 * Proof-of-Concept Box */

#include <Encoder.h>
#include <CheapStepper.h>

#define ENC_STEPS_PER_REV 80.0
Encoder EncO(3,4); // Output Encoder
Encoder EncI(2,1); // Input Encoder

const float GEAR_RATIO = 43.0 / 11.0; // Output to Input Gear Ratio
const float MOT_STEPS_PER_REV = 4075.7728 * GEAR_RATIO;
CheapStepper stepper(8,9,10,11);
bool movingClockwise = true;
unsigned long moveStartTime = 0; // this will save the time (millis()) when we started each new move

// Returns the Output Angle from the Encoder in Degrees
float outputAng(){
  return 360.0 * EncO.read() / ENC_STEPS_PER_REV;
} // #outputAng

// Returns the Input Angle from the Encoder in Degrees
float inputAng(){
  return -360.0 * EncI.read() / ENC_STEPS_PER_REV / GEAR_RATIO;
} // #outputAng

struct CurrentMoveType{
  float start; // Starting Angle of Move [deg]
  float e; // Ending Angle of Move [deg]
  int stepLen; // Number of Steps in Move
} CurrentMove;

// Convert the given number of steps into a angular change in degrees
float stepsToDeg(int n){
  return 360.0 * n / MOT_STEPS_PER_REV;
}

void goTo(float ang){
  goTo(ang, 36);
}

void goTo(float ang, int rpm){
  static float commAng = 0.0; // Last Commanded Angle
  float Da = ang - commAng;
  CurrentMove.start = commAng;
  CurrentMove.e = ang;
  CurrentMove.stepLen = abs(Da) * MOT_STEPS_PER_REV / 360.0;

  //Serial.println(commAng);
  //Serial.println(ang);
  //Serial.println(CurrentMove.stepLen);
  //Serial.println(millis());
  //Serial.println("####");

  movingClockwise = Da < 0;
  moveStartTime = millis();
  stepper.setRpm(rpm);
  stepper.newMove(movingClockwise, CurrentMove.stepLen);
  commAng = ang;
} // #goTo

// Return the Current Commanded Motor Angle using Step Interpolation across
// the Bounds of the Current Move
float getCommAng(){
  return CurrentMove.start + ((float)(CurrentMove.stepLen - abs(stepper.getStepsLeft()))) * (CurrentMove.e-CurrentMove.start) / ((float)CurrentMove.stepLen);
} // #getCommPos

void setup(){
  Serial.begin(9600);
  stepper.set4076StepMode();
  stepper.setRpm(36);
  goTo(180.0);
} // #setup

char dir = 1;
unsigned long last_update = -10000;
float diff = 0.0;
float lag_sum = 0.0;
unsigned long lag_count = 0;
bool at_max = false; // Whether currently at or above max torque (diff)
bool fixing_max = false; // Whether steps are being taken to reduce torque for this max
void loop(){
  stepper.run();
  if(millis() - last_update > 100){
//    Serial.print(outputAng()); Serial.print(", ");
//    Serial.print(inputAng()); Serial.print(", ");
    Serial.println(diff);
//    Serial.print(getCommAng());
//    Serial.print(", ");
//    Serial.println(outputAng());
    last_update = millis(); // Call at e of printing to have event e-start spacing
  }

  lag_sum += outputAng() - inputAng();
  lag_count += 1;
  diff = outputAng() - inputAng() - lag_sum / lag_count;
  at_max = abs(diff) > 30;
  if(fixing_max && !at_max){
    fixing_max = false; // reset. max has been fixed.
  }

  if(at_max && !fixing_max){
    fixing_max = true;
    // Flip direction if displacement opposes direction of motion:
    if(dir != (diff < 0)){
      dir = !dir;
      int steps_remaining = stepper.getStepsLeft();
      stepper.stop();
      goTo((dir ? 1 : -1) * (180.0 - stepsToDeg(steps_remaining)), 50);
    }
  }

  if(stepper.getStepsLeft() == 0 && !fixing_max){
    dir = !dir;
    goTo((dir ? 1 : -1) * 180.0);
  }
} // #loop
