/* Schedule.h
 * Intuitive Scheduling Utility that Allows for Complex Time and Condition Based
 * Behaviors to be Constructed out of Simple, Legible Event-Based Primitives.
 * (admittedly, this has a bit of a ways to go in terms of memory efficiency -
 * badly needs a ring buffer. (especially bad now that state persistence has
 * been added))
 * KNOWN BUGS / PROBLEMS:
 *  - Semi-Required memory leak on the %done% state of Actions. Need to have
 * some way of determining whether / how long other functions will need access to
 * this information after the Action has been deleted. NOTE: Until this is fixed,
 * the ability to create unbounded series of SingleTimedEvents with #in_ is
 * gone. Keep total number of events known and bounded.
 * Author: Connor W. Colombo, 9/21/2018
 * Version: 0.1.4
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

 // Additionally, events which setup other events (using nested actions) return
 // a double pointer to a bool which indicates when all sub-events have been
 // executed at least once.
 // Note: bool** beepboopd must be global.
 beepboopd = sch->IN(3100)->DO_LONG( *(sch->IN(1000)->DO( plt("***BEEP***BOOP***"); )); );
 sch->WHEN(**beepboopd)->DO( plt("## BOP ##"); );
 }
 */

/* NB: Some functionality must be assigned in macros b/c lambdas with captures
 can't be converted to function pointers. */
// More Legible Shorthand for "do_" syntax:
#define DO(x) do_([](){x;})
/* Shorthand for Calling a Function which Takes a Long Time to Complete after it
 Returns (has its own event calls) and returns a double pointer of a boolean which
 indicates when it is done. */
#define DO_LONG(x) \
do_(new NestingAction([](Action* action){ \
delete action->done; \
action->done = x; \
}));
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

typedef bool** ActionState;
#define new_ActionState(b) new bool*(new bool(b));

/*
 * Container for Action which are called in events and their respective metadata.
 */
class Action{ // Abstract Container for Use in Arrays of Pointers
public:
    bool* done = new bool(false);

    virtual ~Action(){
        //delete done; // <- Leave the done state variable behind
        //done = nullptr;
    } // dtor

    virtual void call() = 0;

    /* Tells Whether this Action and its Required Actions are Complete. Returns
     the dereferrenced state of member %done% */
    bool isDone(){
        return *(this->done);
    } // #isDone
}; // class Action
/*
 * Most basic form of an Action which takes a void-void function which has no
 * dependencies and thus is considered to be done executing once the function
 * returns (ie. doesn't generate any Events).
 */
class BasicAction : public Action{
public:
    // Type of Function to be Called which Consumes the Stored Data:
    typedef void (*function) ();

    BasicAction(function f) : oncall{f} {};

    void call(){
        oncall();
        *(this->done) = true;
    }
private:
    // Function to be Executed when this Action is Called:
    function oncall;
}; // class BasicAction
/*
 * Most basic form of an Action which takes a void-Action* function which has
 * dependencies / triggers other events and is expected to set this Action's
 * done value to true once all of its sub-functions are complete.
 */
class NestingAction : public Action{
public:
    // Type of Function to be Called which Consumes the Stored Data:
    typedef void (*function) (Action*);

    NestingAction(function f) : oncall{f} {};

    void call(){
        oncall(this);
    }
private:
    // Function to be Executed when this Action is Called:
    function oncall;
}; // class NestingAction
/*
 * An Action (ie function) to be Performed by being Called when an Event
 * Triggers and Must Receive some Piece(s) of Stored Data of type T to Execute.
 * The contained function is considered to have no dependencies and thus be
 * done executing once the function returns (ie. doesn't generate any Events).
 */
template <typename T>
class DataAction : public Action{
public:
    // Type of Function to be Called which Consumes the Stored Data:
    typedef void (*function) (T);
    // Stored Data to be Given to the Function:
    T data;

    DataAction(function f, T d) :  data{d}, oncall{f} {};

    // Calls this Action by Passing the Stored Data to #oncall and Calling It.
    void call(){
        oncall(data);
        *(this->done) = true;
    }
private:
    // Function to be Executed when this Action is Called:
    function oncall;
}; // Class: DataAction
/*
 * An Action (ie function) to be Performed by being Called when an Event
 * Triggers and Must Receive some Piece(s) of Stored Data of type T to Execute.
 * The contained function has dependencies / triggers other events and is
 * expected to set this Action's done value to true once all of its s
 * sub-functions are complete.
 */
template <typename T>
class NestingDataAction : public Action{
public:
    // Type of Function to be Called which Consumes the Stored Data:
    typedef void (*function) (T, Action*);
    // Stored Data to be Given to the Function:
    T data;

    NestingDataAction(function f, T d) : data{d}, oncall{f} {};

    // Calls this Action by Passing the Stored Data to #oncall and Calling It.
    void call(){
        oncall(this);
    }
private:
    // Function to be Executed when this Action is Called:
    function oncall;
}; // Class: NestingDataAction

/*
 * Basic Event Class which Triggers only when Called Directly.
 */
class Event{
public:
    // Basic void-void function which can signup for the event:
    typedef void (*RegisteredFunction) ();
    const bool runs_once; // Indentifies whether this event only happens once.

    Event() : runs_once{false} {};

    virtual ~Event(){
        /*for(
            std::vector<Action*>::iterator it = this->registry.begin();
            it != this->registry.end();
            ++it
            ){
            delete (*it);
        }
         this->registry.clear(); // TODO: Need to come up with way to make Action::done itself stick around*/
    } // dtor

    /*
     * Request this Event to Execute ASAP.
     * NOTE: Calls happen IN ADDITION to any event-specific timings or conditions. */
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

    /* Add the Given Function to the %registry% as a BasicAction to be Executed
     Every Time the Event is Triggered. Returns a double pointer of the done variable of the Action created. */
    bool** signup(RegisteredFunction fcn){
        Action* a = new BasicAction(fcn);
        this->registry.push_back(a);
        return &(a->done);
    } // #signup

    /* Add the Given Action to the %registry% to be Executed Every Time the Event
     is Triggered. Returns a double pointer of the done variable of the Action. */
    bool** signup(Action* a){
        this->registry.push_back(a);
        return &(a->done);
    } // #signup

    // Alias for Signing Up for the Event
    bool** do_(RegisteredFunction fcn){ return signup(fcn); }
    bool** do_(Action* a){ return signup(a); }

    // Calls All Functions Registered to this Event
    void execute(){
        if(!this->ran || !this->runs_once){
            // Do this ^ check instead of deleting self b/c pointer might be accessed later if in list.
            for(std::vector<Action*>::size_type i = 0; i != this->registry.size(); i++) {
                this->registry[i]->call();
            }
            this->ran = true;
        }
    } // #execute

protected:
    Event(bool ro) : runs_once{ro} {};
    std::vector<Action*> registry;
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
        // Iteration has to account for the fact that elements are intentionally
        // deleted from the vector in the loop and potentially added at any call
        // of #Event::tryExecute
        std::vector<Event*>::size_type size = this->events.size();
        std::vector<Event*>::size_type i = 0;
        while(i < size){
            if( this->events[i]->tryExecute() && this->events[i]->runs_once ){
                // Delete Event if it's been Executed and Only Runs Once
                delete this->events[i]; // Delete the Event
                this->events.erase(this->events.begin() + i); // Remove the addr from the vector
                size--; // As far as we know, the vector is now smaller
            } else{
                ++i; // Increment iterator normally
            }
        }
    } // #loop
}; // Class: Schedule
#endif // SCHEDULE_H
