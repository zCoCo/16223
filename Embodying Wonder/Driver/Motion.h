#ifndef _MOTION_H
#define _MOTION_H
  #include "HAL.h"

  #define MOT_DIR -1 // Used to Invert Motor Direction (-1 for Invert, 1 for Normal)

  // Immediately Set the New Position Target of the Motor to the Given Angle [deg]
  void moveTo(float ang){
    stepper.stop();
    stepper.moveTo(MOT_DIR * ang * MOT_STEPS_PER_REV / 360.0);
  } // #moveTo

  // Immediately Set the New Position Target of the Motor to the Given Angle
  // Relative to the Motor's Current Position [deg]
  void move(float ang){
    stepper.stop();
    stepper.move(MOT_DIR * ang * MOT_STEPS_PER_REV / 360.0);
  } // #move

  // Returns Whether the Motor is Currently Idle (awaiting a new command)
  bool idle(){
    return stepper.distanceToGo() == 0;
  } // #idle

  // Returns the Most Recently Commanded Angle to the Motor
  float getCommAng(){
    return stepper.targetPosition() * 360.0 / MOT_DIR / MOT_STEPS_PER_REV;
  }

  // Perform All Necessary Motion Control Commands:
  void updateMotion(){
    stepper.run();
  } // #updateMotion
#endif // _MOTION_H
