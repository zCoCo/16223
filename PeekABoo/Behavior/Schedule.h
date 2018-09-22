#include <vector>
#include <time.h>
unsigned long millis(){ return 1000 * clock() / CLOCKS_PER_SEC; }
/* Example Usage:
void setup(){
// Basic Call:
  schedule->EVERY(500)->DO(blink()); // Will call #blink every 500ms
  schedule->IN(2500)->DO(doThisOnce()); // Will call #doThisOnce one time in 2.5s
  schedule->WHILE(dist < 10)->DO(swing_arms()); // Will call #swing_arms as often as possible so long as dist < 10.
  schedule->WHEN(dist < 10)->DO(backup()); // Will call #backup every time dist goes from >=10 to <10.

  schedule->EVERY(250)->do_(blink); // if you're just calling a void function with no arguments, it's more effective to just use the lowercase #do_
  schedule->EVERY(100)->DO(x++); // x or other variables accessed must be a global variable

// Or Save Events to be Registered to Later:
  Event* FREQ_100Hz = schedule->EVERY(10);
  Event* TOO_CLOSE = schedule->WHEN(dist < 10);
  // ... somewhere else in code:
  TOO_CLOSE->DO(tone(BUZZER, 1000, 25));
  TOO_CLOSE->SIGNUP(tone(BUZZER, 1000, 25));
}
*/

/* NB: Some functionality must be assigned in macros b/c lambdas with captures
can't be converted to function pointers. */
// More Legible Shorthand for "do_" syntax:
#define DO(x) do_([](){x;})
// More Legible Shorthand for "do_" syntax:
#define SIGNUP(x) signup([](){x;})
// More Legible Shorthand for "while_" syntax:
#define WHILE(x) while_([](){return (x);})
// More Legible Shorthand for "when" syntax
#define WHEN(x) when([](){return (x);})
// Syntax to Normalize All-Caps Syntax used by Conditionals:
#define EVERY(x) every(x)
// Syntax to Normalize All-Caps Syntax used by Conditionals:
#define IN(x) in_(x)

/*
 * Basic Event Class which Triggers if its EventCondition is True When
 * #tryTrigger is Called.
 */
class Event{
public:
  typedef void (*RegisteredFunction) ();

  const bool runs_once; // Indentifies whether this event only happens once.

  Event() : runs_once{false} {};

  virtual ~Event(){} // Destructor

  virtual bool tryTrigger() = 0;

  // Add the Given Function to the %registry% to be Executed Every Time the Event is Triggered
  void signup(RegisteredFunction fcn){
    this->registry.push_back(fcn);
  } // #signup

  // Alias for Signing Up for the Event
  void do_(RegisteredFunction fcn){ signup(fcn); }

  // Call All Functions Registered to this Event
  void trigger(){
    static bool ran = false; // Whether this function has been run before.
    if(!ran || !this->runs_once){
    // Do this ^ check instead of deleting self b/c pointer might be accessed later if in list.
      for(std::vector<RegisteredFunction*>::size_type i = 0; i != this->registry.size(); i++) {
        this->registry[i]();
      }
      ran = true;
    }
  } // #trigger

protected:
    Event(bool ro) : runs_once{ro} {};
    std::vector<RegisteredFunction> registry;
};

/* Event which Triggers Anytime #tryTrigger is called and its condition is True*/
class ConditionalEvent : public Event{
public:
  typedef bool (*EventCondition) ();

  EventCondition condition; // Function that Triggers the Event if it's Ready to be Triggered

  ConditionalEvent(EventCondition t) : condition{t} {}; // Constructor

  virtual ~ConditionalEvent(){
    delete& condition;
  } // Destructor

  /*
   * Triggers this Event if its %condition% Allows It.
   * Returns Whether the Event was Triggered.
   */
  virtual bool tryTrigger(){
    if(this->condition()){
      this->trigger();
      return 1;
    }
    return 0;
  } // #tryTrigger
};

/*
 * Event Class which Triggers when its EventCondition is True When #tryTrigger
 * is Called and was False the Last time it was Called.
 */
class TransitionEvent : public ConditionalEvent{
public:
  TransitionEvent(EventCondition t) : ConditionalEvent(t) {}; // Constructor

  bool tryTrigger(){
    static bool last_state = false;
    bool curr_state = this->condition();

    if(curr_state && !last_state){
      this->trigger();
      last_state = curr_state;
      return 1;
    }

    last_state = curr_state;
    return 0;
  } // #tryTrigger
};

/*
 * Event which Triggers as Close to its Specified Interval after its Previous
 * Execution as Possible
 */
class TimedEvent : public Event{
public:
  unsigned long interval; // Interval between Executions

  TimedEvent(unsigned long i) : interval{i} {}; // Constructor

  virtual ~TimedEvent(){ } // Destructor

  /*
   * Triggers this Event if its %condition% Allows It.
   * Returns Whether the Event was Triggered.
   */
   virtual bool tryTrigger(){
    static unsigned long last_time = millis(); // Grab base time on first call
    static long timer = this->interval;

    unsigned long now = millis();
    timer -= now - last_time;
    last_time = now;

    if(timer < 0){
      timer += this->interval; // Keeps execution freq. as close to interval as possible
      this->trigger();
      return 1;
    }

    return 0;
  } // #tryTrigger

protected:
  TimedEvent(bool runs_once_, unsigned long i) : Event(runs_once_), interval{i} {};
};

class SingleTimedEvent : public TimedEvent{
public:
  SingleTimedEvent(unsigned long i) : TimedEvent(i) {}; // Constructor
};

class Schedule{
public:
  std::vector<Event*> events;

  /* Create an Event to be Triggered as Long as the Given Condition is True */
  ConditionalEvent* while_( bool (*condition)() ){
    ConditionalEvent* e = new ConditionalEvent(condition);
    this->events.push_back(e);
    return e;
  } // #while_

  /* Create an Event to be Triggered Once for Every Time the Given Condition
  Changes from false to true: */
  TransitionEvent* when( bool (*condition)() ){
    TransitionEvent* e = new TransitionEvent(condition);
    this->events.push_back(e);
    return e;
  } // #when

  /* Create an Event that will be Triggered Every %interval% Milliseconds */
  TimedEvent* every(const unsigned long interval){
    TimedEvent* e = new TimedEvent(interval);
    this->events.push_back(e);
    return e;
  } // #every

  /* Create an Event that will be Triggered Once in %t% Milliseconds */
  SingleTimedEvent* in_(const unsigned long t){
    SingleTimedEvent* e = new SingleTimedEvent(t);
    this->events.push_back(e);
    return e;
  } // #in_

  // Function to be Executed on Every Main Loop (as fast as possible)
  void loop(){
    std::vector<Event*>::iterator it;
    for(it = this->events.begin(); it != this->events.end();) {
      if( (*it)->tryTrigger() && (*it)->runs_once ){
        // Delete Event if it's been Executed and Only Runs Once
        delete* it;
        it = this->events.erase(it);
      } else{
        ++it; // Increment iterator normally
      }
    }
  } // #loop

}; // Class: Schedule
