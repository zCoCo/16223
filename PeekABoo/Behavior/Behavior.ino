#include "Schedule.h"
#include "HAL.h"

/* Functional Primitives:
 * NOTE: Left and Right Refer to the Viewer's Left and Right:
 * SENSING:
 * dist() - returns the distance from the observer in cm
 * touched() - returns true while the robot is being touched
 *
 *
 * BEHAVIOUR:
 * blink()                - blinks both eyes once (quick down and up).
 * chuckle()              - chuckles slightly by inverting the eyes three times and moving eye stalks up and down.
 * togglePeek()           - moves hands slightly out of the way of the eyes on first call, on second call it covers them up
 *
 * coverEyes(int time)    - moves both hands over the eyes, taking %time% seconds to do so
 * uncoverEyes(int time)  - uncovers its eyes, taking %time% seconds to do so
 *
 * moveStalks(int percent) - moves stalks to %percent% of the way from the bottom of their swing where 0% is their lowest position and 100% is their highest
 * moveStalkLeft(int percent) - moves left stalk to %percent% of the way from the bottom of its swing where 0% is its lowest position and 100% is its highest
 * moveStalkRight(int percent) - moves right stalk to %percent% of the way from the bottom of its swing where 0% is its lowest position and 100% is its highest
 *
 * moveHands(int percent) - moves hands to %percent% of the way from the bottom of their swing where 0% is their lowest position and 100% is their highest
 * moveHandLeft(int percent) - moves left hand to %percent% of the way from the bottom of its swing where 0% is its lowest position and 100% is its highest
 * moveHandRight(int percent) - moves right hand to %percent% of the way from the bottom of its swing where 0% is its lowest position and 100% is its highest
 */

 /* EXAMPLE OF ALL TIMING OPTIONS (put these in setup NOT loop)
 ** avoid calling variables directly from inside these functions unless they are global variables **

 sch->EVERY(500)->DO(blink()); // Will call #blink every 500ms
 sch->EVERY_WHILE(750, dist() < 10)->DO(togglePeek()); // Will peek / unpeek Every 750ms while Dist is < 10cm

 sch->IN(2500)->DO(doThisOnce()); // Will call #doThisOnce one time in 2.5s

 sch->WHILE(dist() < 10)->DO(swing_arms()); // Will call #swing_arms (not a real function) as often as possible as long as dist() < 10.
 sch->WHEN(dist() > 10)->DO(someOtherThing()); // Will call #backup every time dist goes from <=10 to >10.
 sch->WHEN(touched())->DO(uncoverEyes()); // Will uncover eyes when touched goes from false to true (so, when touched)

 // Other sometimes more efficient notation:
 sch->EVERY(250)->do_(blink); // if you're just calling a void function with no arguments, it's more effective to just use the lowercase #do_
 sch->EVERY(100)->DO(x++); // x or other variables accessed must be a global variables
 */

 // STATE VARIABLES:
 struct RobotType{
   bool* awake = new bool(false);
   bool* eyes_open = new bool(false);
 } Robot;

bool* wakeUp(){
  eyeLids(100); // Eyes Start Closed (call this before the scheduler turns on)
  Robot.eyes_open = sch->IN(500)->DO_LONG(blink(750));
  return sch
    ->WHEN( *(Robot.eyes_open) )
    ->DO([](){
      moveEyeLidsTo(35);
      *(Robot.awake) = true;
    });
} // #wakeUp

void setup(){
  Serial.begin(9600);
  initHAL();
  moveStalks(0);
  moveHands(0);

  wakeUp();
  sch->WHEN( *(Robot.awake) )->DO(chuckle());

  sch->WHEN( true )->DO(Serial.println(PSTR("Hi!")));
  sch->WHEN( *(Robot.eyes_open) )->DO(Serial.print(PSTR("Waking Up - ")); Serial.println(millis()));
  sch->WHEN( *(Robot.awake) )->DO(Serial.print(PSTR("I'm Awake - ")); Serial.println(millis()));

  //sch->EVERY(1500)->DO(Serial.print("Ping - "); Serial.println(millis()));
} // #setup

void loop(){
  sch->loop();
} // #loop
