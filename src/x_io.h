#ifndef X_IO_H
#define X_IO_H

#include <ESP_FlexyStepper.h>
#include "dc_alert.h"
#include "x_models.h"


/* TIMERS *********************************************************************************************/

#define ITR_DEBOUNCE_TIMER 0 // Timer 0
#define ITR_DEBOUNCE_TIMER_PRESCALE 80 // Prescale of 80 = 1MHz for ESP32 DevKit V1
#define ITR_DEBOUNCE_TIMER_EDGE true // No idea what this is; must be set true...
#define ITR_DEBOUNCE_TIMER_AUTORUN true // set 'autoreload' true to run continuously
#define ITR_DEBOUNCE_TIMER_RUN_ONCE false // set 'autoreload' false to run once
#define ITR_DEBOUNCE_TIMER_COUNT_UP true // Timer counts up as opposed to down
#define ITR_DEBOUNCE_TIMER_PERIOD_uSEC 2000
extern void changeITRDebounceTimerPeriod();

#define OPS_HAMMER_TIMER 1                          // Timer 1
#define OPS_HAMMER_TIMER_PRESCALE 80                // Prescale of 80 = 1MHz for ESP32 DevKit V1
#define OPS_HAMMER_TIMER_EDGE true                  // No idea what this is; must be set true...
#define OPS_HAMMER_TIMER_AUTORUN true               // set 'autoreload' true to run continuously  
#define OPS_HAMMER_TIMER_RUN_ONCE false             // set 'autoreload' false to run once
#define OPS_HAMMER_TIMER_COUNT_UP true              // Timer counts up as opposed to down
#define OPS_HAMMER_TIMER_STRIKE_PERIOD_uSEC 500000  // 0.5 seconds for hammer to drop
extern void startHammerTimer();
extern void changeHammerTimeroutPeriod();

// #define OPS_BRAKE_TIMER 2                           // Timer 2
// #define OPS_BRAKE_TIMER_PRESCALE 80                 // Prescale of 80 = 1MHz for ESP32 DevKit V1
// #define OPS_BRAKE_TIMER_EDGE true                   // No idea what this is; must be set true...
// #define OPS_BRAKE_TIMER_RUN_ONCE false              // set 'autoreload' false to run once
// #define OPS_BRAKE_TIMER_COUNT_UP true               // Timer counts up as opposed to down
// #define OPS_BRAKE_TIMER_ON_PERIOD_uSEC 300000       // 0.3 seconds for pressure to drop
// #define OPS_BRAKE_TIMER_OFF_PERIOD_uSEC 1000000     // 1.0 seconds for pressure to build


/* INTERRUPTS *****************************************************************************************/
#define PIN_ITR_ESTOP 3                 // Interrupt -> Input Pullup -> Low = Emergency stop button pressed
#define PIN_ITR_DOOR 21                 // Interrupt -> Input Pullup -> Low = Door is closed
#define PIN_ITR_FIST 19                 // Interrupt -> Input Pullup -> Low = Fist is in contact with hammer
#define PIN_ITR_ANVIL 18                // Interrupt -> Input Pullup -> Low = Hammer is in contact with anvil 
#define PIN_ITR_HOME 5                 // Interrupt -> Input Pullup -> Low = Counter-weight is at the top of it's stroke 
#define PIN_ITR_TOP 17                  // Interrupt -> Input Pullup -> Low = Fist is at top 
#define PIN_ITR_PRESSURE 16             // Interrupt -> Input Pullup -> Low = Brake has pressure 

#define ITR_PIN_ACTIVE_LOW true
#define ITR_PIN_ACTIVE_HIGH false
#define ITR_DEBOUNCE_PERIOD_mSEC 5

// extern uint32_t g_ui32InterruptFlag;

#define ITR_PIN_COUNT 7
typedef enum { 
    CHK_ESTOP = 0, 
    CHK_DOOR, 
    CHK_FIST, 
    CHK_ANVIL,
    CHK_TOP,
    CHK_PRESSURE 
} eItrCheckMap_t;

typedef void (*isrCallBack) ();

/*  All of our interrupt input pins are: 
    - pulled HIGH internally, 
    - conneted to normally open switches,
    - pulled LOW when their contact closes

    On READ, if we want: 
    - a CLOSED contact (LOW) to mean TRUE --> we want ACTIVE LOW  
    - an OPEN contact (HIGH) to mean TRUE --> we want ACTIVE HIGH */
class ITRPin {

private:
    bool bPrevState;                            // Used to detect pin state change after debounce

public:
    uint8_t pin;                                // GPIO Pin number: Required at initialization 
    bool *pbPinState;                           // &(bool)State.memberName: Required at initialization 
    bool bActiveLow;                            // Ativation state: Default --> ITR_PIN_ACTIVE_LOW
    uint32_t checkTime;                         // Flag and wait time: To clear set to --> 0
    uint32_t debouncePeriod;                    // Debounce period: Default --> ITR_DEBOUNCE_PERIOD_mSEC

    ITRPin( 
        uint8_t p, 
        bool *ps,                               
        bool a = ITR_PIN_ACTIVE_LOW,            // Activation: Default LOW unless overridden     
        uint32_t dbt = ITR_DEBOUNCE_PERIOD_mSEC // Debounce period: Default --> ITR_DEBOUNCE_PERIOD_mSEC
    ) {
        pin = p;
        pbPinState = ps;
        bActiveLow = a;
        checkTime = 0;                          
        debouncePeriod = dbt;
    }

    void setupPin(isrCallBack func) {
        pinMode(pin, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(pin), func, CHANGE);
    }

    void checkPin() {

        *pbPinState = ( bActiveLow                  
            ? !(bool)digitalRead(pin)           // ACTIVE LOW --> TRUE when contact is CLOSED
            : (bool)digitalRead(pin)            // ACTIVE HIGH --> TRUE when contact is OPEN
        );
    }

    /*  Called by isrDebounceFunce():
        - If ITRPin.checkTime has passed 
        - And ITRPin.bpPinState has changed
        - Sets global interrupt flag */
    void itrCheck(uint32_t nowMillis) {

        if( checkTime == 0                      /* The interrupt is quiet */
        ||  checkTime > nowMillis               /* The debouncce period has yet to pass */
        )   return;                             // Get outta here
        
        else {                                  /* Check on this pin */

            bPrevState = *pbPinState;           // What state do we think this pin is in now?

            checkPin();                         // What is the actual pin state?

            if( bPrevState != *pbPinState       /* The pin state changed */
            )   g_state.interruptFlag = true;   //g_ui32InterruptFlag = 1;        // Ring the bell!
            
            checkTime = 0;                      // Reset the time / flag
        }
    }

};

/* INTERRUPTS *** END ********************************************************************************/



/* RELAY CONTROL *************************************************************************************/
#define PIN_OUT_BRAKE 23                        // Relay Control -> Output -> Low = Brake is engaged
#define PIN_OUT_MAGNET 22                       // Relay Control -> Output -> Low = Brake is engaged
#define PIN_OUT_MOT_DIR 12                      // Motor Direction  -> Output -> HIGH = ???
#define PIN_OUT_MOT_STEP 14                     // Motor Move       -> Output -> One pulse per step

#define DOUT_PIN_ACTIVE_LOW true
#define DOUT_PIN_ACTIVE_HIGH false

#define OUT_PIN_COUNT 4
typedef enum {
    DOUT_BRAKE = 0,
    DOUT_MAGNET,
    DOUT_MOT_STEP,
    DOUT_MOT_DIR
} eDOUTPinMap_t;

class DOUTPin {
private:
    bool bHasReadableState;                 // Correspondsd to State.memberName: Default --> true

public:
    uint8_t pin;                            // GPIO Pin number: Required at initialization 
    bool *pbPinState;                       // &(bool)State.memberName 
    bool bActiveLow;                        // Ativation state: Default --> DOUT_PIN_ACTIVE_LOW

    DOUTPin( 
        uint8_t p, 
        bool *ps,                           // &(bool)State.memberName            
        bool a = DOUT_PIN_ACTIVE_LOW,       // Activation: Default --> DOUT_PIN_ACTIVE_LOW 
        bool hrs = true                     // Correspondsd to State.memberName: Default --> true
    ) {
        pin = p;
        pbPinState = ps;
        bActiveLow = a;
        bHasReadableState = hrs;
    }

    void setupPin() {
        pinMode(pin, OUTPUT);
    }

    void checkPin() {

        if( bHasReadableState               /* This pin has a corresponding &(bool)State.memberName */
        ) {
            *pbPinState = ( bActiveLow                  
                ? !(bool)digitalRead(pin)   // ACTIVE LOW --> TRUE (ENABLED) when pin is LOW
                : (bool)digitalRead(pin)    // ACTIVE HIGH --> TRUE (ENABLED) when pin is HIGH
            );
        }
    }

    void enable() {

        digitalWrite(pin, ( bActiveLow      
            ? LOW                           // ACTIVE LOW --> TRUE (ENABLED) when pin is LOW
            : HIGH                          // ACTIVE HIGH --> TRUE (ENABLED) when pin is HIGH
        ));
        checkPin();                         // Make sure we know what state the pin is in now
        /* TODO: ERROR ON FAILURE */
    }

    void disable() {

        digitalWrite(pin, ( bActiveLow 
            ? HIGH                      // ACTIVE LOW --> FALSE (DISNABLED) when pin is HIGH
            : LOW                       // ACTIVE HIGH --> FALSE (DISNABLED) when pin is LOW
        ));
        checkPin();
    }

};


extern void brakeOn();
extern void brakeOff();

extern void magnetOn();
extern void magnetOff();


/* RELAY CONTROL *** END ***************************************************************************/



/* MOTOR CONTROL ***********************************************************************************/

#define MOT_DIR_UP HIGH
#define MOT_DIR_DOWN LOW
#define MOT_SERVICE_CORE 1 // Which processor core onwhich to run the stepper service 

extern Alert* motorSetPositionAsZero();
extern Alert* motorGetPosition();
extern Alert* motorSetSpeed(uint32_t stepsPerSec);
extern void motorSetCourse(int32_t steps);
extern bool motorTargetReached();
extern void motorStop();

/* MOTOR CONTROL *** END *************************************************************************/


extern void checkStateIOPins();
extern void setupIO();


#endif /* X_IO_H */