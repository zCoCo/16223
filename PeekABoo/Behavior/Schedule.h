#include <time.h>
unsigned long millis(){ return 1000 * clock() / CLOCKS_PER_SEC; }
/* Example Usage:
void setup(){
// Basic Call:
  schedule->EVERY(500)->DO(blink());
  schedule->EVERY(100)->DO(update(dist));
  schedule->WHILE(dist < 10)->DO(swing_arms());
  schedule->WHEN(dist < 10)->DO(backup());

// Or Save Events to be Registered to Later:
  Event* FREQ_100Hz = schedule->EVERY(10);
  Event* TOO_CLOSE = schedule->WHEN(dist < 10);
  // ... somewhere else in code:
  TOO_CLOSE->DO(tone(BUZZER, 1000, 25));
}
*/

/* NB: Some functionality must be assigned in macros b/c lambdas with captures
can't be converted to function pointers. */
// More Legible Shorthand for "do_" syntax:
#define DO(x) do_([](){x;})
// More Legible Shorthand for "while_" syntax:
#define WHILE(x) while_([](){return (x);})

/* Create an Event to be Triggered Once for Every Time the Given Condition
Changes from false to true: */
#define WHEN(x) while_([](){ \
  static bool last_check = false; /* State on the Previous Check */ \
\
  bool current = (x); \
\
  if(current && !last_check){\
    last_check = current;\
    return 1;\
  }\
\
  last_check = current;\
  return 0;\
})

// Syntax to Normalize All-Caps Syntax for WHEN:
#define EVERY(x) every(x)

class Event{
public:
  typedef bool (*EventCondition) ();
  typedef void (*RegisteredFunction) ();

  EventCondition condition; // Function that Triggers the Event if it's Ready to be Triggered

  Event(EventCondition t) : condition(t) {}; // Constructor

  /*
   * Triggers this Event if its %condition% Allows It.
   * Returns Whether the Event was Triggered.
   */
  bool tryTrigger(){
    if(this->condition()){
      this->trigger();
      return 1;
    }
    return 0;
  }

  // Add the Given Function to the %registry% to be Executed Every Time the Event is Triggered
  void do_(RegisteredFunction fcn){
    this->registry.push_back(fcn);
  } // #do_

  // Call All Functions Registered to this Event
  void trigger(){
    for(std::vector<RegisteredFunction*>::size_type i = 0; i != this->registry.size(); i++) {
      this->registry[i]();
    }
  } // #trigger

  private:
    std::vector<RegisteredFunction> registry;
};

class Schedule{
public:
  std::vector<Event*> events;

  /* Create an Event to be Triggered as Long as the Given Condition is True */
  Event* while_( bool (*condition)() ){
    Event* e = new Event(condition);
    this->events.push_back(e);
    return e;
  } // #while_


  /* Create an Event that will be Triggered every %interval% Milliseconds */
  Event* every(const unsigned long interval){
    Event* e = new Event([](Event* evnt){
      static unsigned long last_time = millis(); // Time of last trigger
      static unsigned long i = interval;

      if(millis() - last_time > i){
        last_time = millis();
        evnt->trigger();
        return 1;
      }

      return 0;
    });
    this->events.push_back(e);
    return e;
  } // #every

  // Function to be Executed on Every Main Loop (as fast as possible)
  void loop(){
    for(std::vector<Event*>::size_type i = 0; i != this->events.size(); i++) {
      this->events[i]->tryTrigger(this->events[i]);
    }
  } // #loop

}; // Class: Schedule

Schedule* schedule = new Schedule();
