/* Basic Code Base for Verifying All Core Functionality on a
 * Proof-of-Concept Box */

#include <Encoder.h>
#include <CheapStepper.h>

#define ENC_STEPS_PER_REV 80.0
Encoder EncO(3,4); // Output Encoder

const float MOT_STEPS_PER_REV = 4075.7728 * 43.0 / 11.0;
CheapStepper stepper(8,9,10,11);
bool movingClockwise = true;
unsigned long moveStartTime = 0; // this will save the time (millis()) when we started each new move

// Returns the Output Angle of the Encoder in Degrees
float outputAng(){
  return 360 * EncO.read() / ENC_STEPS_PER_REV;
} // #outputAng

struct CurrentMoveType{
  float start; // Starting Angle of Move [deg]
  float e; // Ending Angle of Move [deg]
  int stepLen; // Number of Steps in Move
} CurrentMove;

void goTo(float ang){
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

void loop(){
  stepper.run();
  if(millis() - last_update > 100){
    Serial.print(getCommAng());
    Serial.print(", ");
    Serial.println(outputAng());
    last_update = millis(); // Call at e of printing to have event e-start spacing
  }
  if(stepper.getStepsLeft() == 0){
    dir = !dir;
    goTo((dir ? 1 : -1) * 180.0);
  }
} // #loop
