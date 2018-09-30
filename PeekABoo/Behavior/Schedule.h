/* Schedule.h
 * Intuitive Scheduling Utility that Allows for Complex Time and Condition Based
 * Behaviors to be Constructed out of Simple, Legible Event-Based Primitives.
 * (admittedly, this has a bit of a ways to go in terms of memory efficiency -
 * badly needs a ring buffer.)
 * Author: Connor W. Colombo, 9/21/2018
 * Version: 0.1.0
 * License: MIT
 */
#ifndef SCHEDULE_H
#define SCHEDULE_H
#include <StandardCplusplus.h>
#include <vector>
/* Example Usage (only call these once, likely in setup):
 ** avoid calling variables directly from inside these functions unless they are global variables **

void setup(){
// Basic Call:
 sch->EVERY(500)->DO(blink()); // Will call #blink every 500ms
 sch->EVERY_WHILE(750, dist < 10)->DO(togglePeek()); // Will peek / unpeek every 750ms while dist is < 10cm

 sch->IN(2500)->DO(doThisOnce()); // Will call #doThisOnce one time in 2.5s

 sch->NOW->DO(sortOfUrgent()); // Will call #sortOfUrgent as soon as possible without blocking other events (useful in comm. interrupts for longer behavior)

 sch->WHILE(dist < 10)->DO(swing_arms()); // Will call #swing_arms as often as possible as long as dist < 10.
 sch->WHEN(dist > 10)->DO(someOtherThing()); // Will call #someOtherThing every time dist goes from <=10 to >10.
 sch->WHEN(touched())->DO(uncoverEyes()); // Will uncover eyes when touched goes from false to true (so, when touched)

 // Other more efficient notation for simple function calls:
 sch->EVERY(250)->do_(blink); // if you're just calling a void function with no arguments, it's more effective to just use the lowercase #do_
 // Note:
 sch->EVERY(100)->DO(x++); // x or other variables accessed directly must be a global variables (not local scope)

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
// More Legible Shorthand for "everyWhile" syntax:
#define EVERY_WHILE(x,y) everyWhile(x, [](){return (y);})
// Syntax to Normalize All-Caps Syntax used by Conditionals:
#define IN(x) in_(x)
// Shorthand Syntax for Performing a Task as Soon as Possible:
#define NOW in_(0)

/*
 * Container for Information about the State of an Event which Allows its
 * Information to be Altered by Nested Events so that Complex Networks of
 * Dependencies for Whether an Event is "done" can be Built if Desired. The
 * primary reason for this class is so that event's whose registed functions
 * establish other events which won't execute until later can ...*/
struct EventState{
  bool* done; // Whether the event's function and all of its nested actions have completed at least once
  EventState() : done(new bool(false)) {}; //ctor
  ~EventState(){
    delete done;
  } // dtor
};

/*
 * An Action (ie function) to be Performed by being Called when an Event
 * Triggers and Must Receive some Piece(s) of Stored Data of type T to Execute.
 */
 class DataActionType{ // Abstract Container for Use in Arrays of Pointers
 public:
   virtual ~DataActionType() = 0;
   virtual void call() = 0;
 };
 template <typename T>
 class DataAction : public DataActionType{
  public:
    // Type of Function to be Called which Consumes the Stored Data:
    typedef void (*function) (T);
    // Function to be Executed when this Action is Called:
    function oncall;
    // Stored Data to be Given to the Function:
    T data;

    DataAction(function f, T d) : oncall{f}, data{d} {};
    virtual ~DataAction(){
      delete& oncall;
    }

    // Calls this Action by Passing the Stored Data to #oncall and Calling It.
    void call(){
      oncall(data);
    }
 }; // Class: DataAction

/*
 * Basic Event Class which Triggers only when Called Directly.
 */
class Event{
public:
  typedef void (*RegisteredFunction) ();
  // Special Type of Registered Function which takes a bool* which it sets to true once all its actions have been performed:
  typedef void (*LayeredRegisteredFunction) (bool*);

  EventState* state = new EventState(); // Contains Information about the Event's Executio State

  const bool runs_once; // Indentifies whether this event only happens once.

  Event() : runs_once{false} {};

  virtual ~Event(){} // Destructor

  /*
   * Request this Event to Execute ASAP.
   * NOTE: Calls happen in addition to any event-specific timings or conditions. */
  void call(){
    this->calledButNotRun = true;
  } // #call

  /*
   * Executes this Event if it Should Execute either Because it's been Called or
   * Should Self-Trigger.
   * Returns Whether the Event was Executed.
   */
  bool tryExecute(){
    if(this->shouldTrigger() || this->calledButNotRun){ // Call #shouldTrigger first
      this->execute();
      this->calledButNotRun = false;
      return 1;
    }

    return 0;
  } // #tryExecute

  /* Test if this Event Should Self-Trigger*/
  virtual bool shouldTrigger(){
    return 0; // Basic Events only Trigger when Explicitly Called
  } // #shouldTrigger

  /* Add the Given Function to the %registry% to be Executed Every Time the
  Event is Triggered. Returns a bool* whose deref value becomes true when
  then event has been called at least once (is done executing) */
  bool* signup(RegisteredFunction fcn){
    this->registry.push_back(fcn);
    return &this->ran;
  } // #signup
  bool* signup(LayeredRegisteredFunction fcn){
    bool* b = new bool(false);
    this->registry.push_back(fcn);
    return &this->ran;
  } // #signup
  bool* signup(DataActionType* da){
    this->dataActionRegistry.push_back(da);
    return &this->ran;
  } // #signup

  // Alias for Signing Up for the Event
  bool* do_(RegisteredFunction fcn){ return signup(fcn); }
  bool* do_(LayeredRegisteredFunction fcn){ return signup(fcn); }
  bool* do_(DataActionType* da){ return signup(da); }

  // Calls All Functions Registered to this Event
  void execute(){
    if(!this->ran || !this->runs_once){
    // Do this ^ check instead of deleting self b/c pointer might be accessed later if in list.
      for(std::vector<RegisteredFunction*>::size_type i = 0; i != this->registry.size(); i++) {
        this->registry[i]();
      }
      for(std::vector<DataActionType*>::size_type i = 0; i != this->dataActionRegistry.size(); i++) {
        this->dataActionRegistry[i]->call();
      }
      this->ran = true;
    }
  } // #execute

protected:
  Event(bool ro) : runs_once{ro} {};
  std::vector<RegisteredFunction> registry;
  std::vector<LayeredRegisteredFunction> layeredRegistry;
  std::vector<DataActionType*> dataActionRegistry;
  bool ran = false; // Whether this function has been run before (ever).
  bool calledButNotRun = false; // Whether this Event has been Called Recently but Not Yet Executed
}; // Class: Event

/* Event which Triggers Anytime #shouldTrigger is called and its condition is True*/
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
  virtual bool shouldTrigger(){
    if(this->condition()){
      return 1;
    }
    return 0;
  } // #shouldTrigger
};

/*
 * Event Class which Triggers when its EventCondition is True When #shouldTrigger
 * is Called and was False the Last time it was Called.
 */
class TransitionEvent : public ConditionalEvent{
public:
  TransitionEvent(EventCondition t) : ConditionalEvent(t) {}; // Constructor

  bool shouldTrigger(){
    bool curr_state = this->condition();

    if(curr_state && !this->last_state){
      this->last_state = curr_state;
      return 1;
    }

    this->last_state = curr_state;
    return 0;
  } // #shouldTrigger

protected:
  bool last_state = false;
};

/*
 * Event which Triggers as Close to its Specified Interval after its Previous
 * Execution as Possible
 */
class TimedEvent : public Event{
public:
  unsigned long interval; // Interval between Executions

  TimedEvent(unsigned long i) : interval{i} {
    this->timer = i;
    this->last_time = millis();
  }; // Constructor

  ~TimedEvent(){ } // Destructor

  /*
   * Triggers this Event if its %condition% Allows It.
   * Returns Whether the Event was Triggered.
   */
   bool shouldTrigger(){
      unsigned long now = millis();
      this->timer -= now - last_time;
      this->last_time = now;

      if(this->timer < 0){
        this->timer += this->interval; // Keeps execution freq. as close to interval as possible
        return 1;
      }

      return 0;
    }  // #shouldTrigger

protected:
  unsigned long last_time;
  long timer;
  TimedEvent(bool runs_once_, unsigned long i) : Event(runs_once_), interval{i} {
    this->timer = i;
    this->last_time = millis();
  };
};

/* An Event which Triggers Once After a Set Period of Time */
class SingleTimedEvent : public TimedEvent{
public:
  SingleTimedEvent(unsigned long i) : TimedEvent(true, i) {}; // Constructor
};

/* An Event which Triggers at a Certain Frequency so Long as a Given Condition is True */
class ConditionalTimedEvent : public TimedEvent{
public:
  typedef bool (*EventCondition) ();

  EventCondition condition; // Function that Triggers the Event if it's Ready to be Triggered

  ConditionalTimedEvent(unsigned long i, EventCondition t) : TimedEvent(i), condition(t){};

  virtual ~ConditionalTimedEvent(){
    delete& condition;
  } // Destructor

  /*
   * Triggers this Event if its %condition% Allows It.
   * Returns Whether the Event was Triggered.
   */
   bool shouldTrigger(){
      unsigned long now = millis();
      this->timer -= now - last_time;
      this->last_time = now;

      bool curr_state = this->condition();

      // Everytime Condition Becomes True, Restart Timer
      if(curr_state && !this->last_state){
        timer = this->interval;
      }

      this->last_state = curr_state;

      if(curr_state && this->timer < 0){
        this->timer += this->interval; // Keeps execution freq. as close to interval as possible
        return 1;
      }

      return 0;
    }  // #shouldTrigger

protected:
  bool last_state = false;
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

  /*
   * Create an Event that will be Triggered Every %interval% Milliseconds While
   * a Given Condition is True, starting %interval% Milliseconds AFTER the
   * Condition Becomes True.
   */
  ConditionalTimedEvent* everyWhile(const unsigned long interval, bool (*condition)()){
    ConditionalTimedEvent* e = new ConditionalTimedEvent(interval, condition);
    this->events.push_back(e);
    return e;
  } // #everyWhile

  // Function to be Executed on Every Main Loop (as fast as possible)
  void loop(){
    std::vector<Event*>::iterator it;
    for(it = this->events.begin(); it != this->events.end();) {
      if( (*it)->tryExecute() && (*it)->runs_once ){
        // Delete Event if it's been Executed and Only Runs Once
        delete* it;
        it = this->events.erase(it);
      } else{
        ++it; // Increment iterator normally
      }
    }
  } // #loop

}; // Class: Schedule
#endif // SCHEDULE_H
