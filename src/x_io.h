#ifndef X_IO_H
#define X_IO_H

#include <ESP_FlexyStepper.h>
#include "x_models.h"


/* INTERRUPTS *****************************************************************************************/
#define PIN_ITR_ESTOP 4                 // Interrupt -> Input Pullup -> Low = Emergency stop button pressed
#define PIN_ITR_DOOR 16                 // Interrupt -> Input Pullup -> Low = Door is closed
#define PIN_ITR_FIST 34                 // Interrupt -> Input Pullup -> Low = Fist is in contact with hammer
#define PIN_ITR_ANVIL 35                // Interrupt -> Input Pullup -> Low = Hammer is in contact with anvil 
#define PIN_ITR_HOME 17                 // Interrupt -> Input Pullup -> Low = Counter-weight is at the top of it's stroke 
#define PIN_ITR_TOP 32                  // Interrupt -> Input Pullup -> Low = Fist is at top 
#define PIN_ITR_PRESSURE 33             // Interrupt -> Input Pullup -> Low = Brake has pressure 

#define ITR_PIN_ACTIVE_LOW true
#define ITR_PIN_ACTIVE_HIGH false

/* INTERRUPT PINS -> DEBOUNCE */
#define ITR_DEBOUNCE_PERIOD_mSEC 5
#define ITR_DEBOUNCE_TIMER_PERIOD_uSEC 2000
#define ITR_DEBOUNCE_TIMER 0            // Timer 0
#define ITR_DEBOUNCE_PRESCALE 80        // Prescale of 80 = 1MHz for ESP32 DevKit V1
#define ITR_DEBOUNCE_COUNT_UP true      // Timer counts up as opposed to down
#define ITR_DEBOUNCE_EDGE true          // No idea what this is; must be set true...
#define ITR_DEBOUNCE_AUTORUN true       // set 'autoreload' true to run continuously
#define ITR_DEBOUNCE_RUN_ONCE false     // set 'autoreload' false to run once

extern bool g_ui32InterruptFlag;

#define N_ITR_PINS 7
typedef enum { 
    CHK_ESTOP = 0, 
    CHK_DOOR, 
    CHK_FIST, 
    CHK_ANVIL,
    CHK_HOME,
    CHK_TOP,
    CHK_PRESSURE 
} eItrCheckMap_t;

/*  All of our interrupt input pins are: 
    - pulled HIGH internally, 
    - conneted to normally open switches,
    - pulled LOW when their contact closes

    On READ, if we want: 
    - a CLOSED contact (LOW) to mean TRUE --> we want ACTIVE LOW  
    - an OPEN contact (HIGH) to mean TRUE --> we want ACTIVE HIGH */
class DINPin {

private:
    bool bPrevState;                            // Used to detect pin state change after debounce

public:
    uint8_t pin;                                // GPIO Pin number: Required at initialization 
    bool *pbPinState;                           // &(bool)State.memberName: Required at initialization 
    bool bActiveLow;                            // Ativation state: Default --> ITR_PIN_ACTIVE_LOW
    uint32_t checkTime;                         // Flag and wait time: To clear set to --> 0
    uint32_t debouncePeriod;                    // Debounce period: Default --> ITR_DEBOUNCE_PERIOD_mSEC

    DINPin( 
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

    void checkPin() {

        *pbPinState = ( bActiveLow                  
            ? !(bool)digitalRead(pin)       // ACTIVE LOW --> TRUE when contact is CLOSED
            : (bool)digitalRead(pin)        // ACTIVE HIGH --> TRUE when contact is OPEN
        );
    }

    /*  Called by isrDebounceFunce():
        - If DINPin.checkTime has passed 
        - And DINPin.bpPinState has changed
        - Sets global interrupt flag */
    void itrCheck(uint32_t nowMillis) {

        if( checkTime > 0                       /* The interrupt fired */
        &&  checkTime < nowMillis               /* The debouncce period has ended */
        ) {

            bPrevState = *pbPinState;           // What state do we think this pin is in now?

            checkPin();                         // What is the actual pin state?

            if( bPrevState != *pbPinState       /* The pin state changed */
            ) {
                g_ui32InterruptFlag = true;     // Ring the bell!
            }

            checkTime = 0;                      // Reset the time / flag
        }
    }

};

typedef void (*isrCallBack) ();
typedef struct {DINPin obj; isrCallBack func;} ITRPin;
extern ITRPin itrPins[N_ITR_PINS];

void IRAM_ATTR debounce(eItrCheckMap_t eChk);

/* INTERRUPTS *** END ********************************************************************************/



/* OUTPUT CONTROL ************************************************************************************/
#define PIN_OUT_BRAKE 23                // Relay Control -> Output -> Low = Brake is engaged
#define PIN_OUT_MAGNET 22               // Relay Control -> Output -> High = Brake is engaged
#define PIN_OUT_MOT_EN 13               // Relay Control -> Output -> Low = Motor is Enabled
#define PIN_OUT_MOT_STEP 26             // Stepper Control -> Output -> One pulse per step
#define PIN_OUT_MOT_DIR 25              // Stepper Control -> Output -> High = Up

#define DOUT_PIN_ACTIVE_LOW true
#define DOUT_PIN_ACTIVE_HIGH false

#define N_DOUT_PINS 5
typedef enum {
    DOUT_BRAKE = 0,
    DOUT_MAGNET,
    DOUT_MOTOR,
    DOUT_MOT_STEP,
    DOUT_MOT_DIR
} eDOUTPinMap_t;

class DOUTPin {

public:
    uint8_t pin;                            // GPIO Pin number: Required at initialization 
    bool *pbPinState;                       // &(bool)State.memberName: Default --> NULL 
    bool bActiveLow;                        // Ativation state: Default --> DOUT_PIN_ACTIVE_LOW

    DOUTPin( 
        uint8_t p, 
        bool *ps = NULL,                    // &(bool)State.memberName: Default --> NULL            
        bool a = DOUT_PIN_ACTIVE_LOW        // Activation: Default --> DOUT_PIN_ACTIVE_LOW    
    ) {
        pin = p;
        pbPinState = ps;
        bActiveLow = a;
    }

    void checkPin() {

        if( *pbPinState                     /* This pin has a corresponding &(bool)State.memberName */
        ) {
            *pbPinState = ( bActiveLow                  
                ? !(bool)digitalRead(pin)   // ACTIVE LOW --> TRUE (ENABLED) when pin is LOW
                : (bool)digitalRead(pin)    // ACTIVE HIGH --> TRUE (ENABLED) when pin is HIGH
            );
        }
    }

    void enable() {

        digitalWrite(pin, ( bActiveLow 
            ? LOW                       // ACTIVE LOW --> TRUE (ENABLED) when pin is LOW
            : HIGH                      // ACTIVE HIGH --> TRUE (ENABLED) when pin is HIGH
        ));
        checkPin();

    }

    void disable() {

        digitalWrite(pin, ( bActiveLow 
            ? HIGH                      // ACTIVE LOW --> FALSE (DISNABLED) when pin is HIGH
            : LOW                       // ACTIVE HIGH --> FALSE (DISNABLED) when pin is LOW
        ));
        checkPin();
    }

};

extern DOUTPin outPins[N_DOUT_PINS];

/* OUTPUT CONTROL *** END **************************************************************************/



/* MOTOR CONTROL ***********************************************************************************/

#define MOT_DIR_UP HIGH
#define MOT_DIR_DOWN LOW
#define MOT_STEPS_PER_SEC_LOW 500
#define MOT_STEPS_PER_SEC_HIGH 4000
#define MOT_STEPS_PER_SEC_ACCEL 2000
#define MOT_SERVICE_CORE 0 // Which processor core onwhich to run the stepper service 

#define MOT_RECOVERY_STEPS -500 // 1/4 Revolution
#define MOT_STEP_PER_REV 2000

#define MOT_INCH_PER_REV 6.000
#define MOT_HEIGHT_MAX_INCH 48.000
#define MOT_HEIGHT_MAX_STEP 16000 // = ( 48 / 6 ) * 2000

extern void motorSetPositionAsZero();
extern bool motorTargetReached();
extern void motorGetPosition();
extern void motorSetSpeed(int stepsPerSec);
extern void motorMoveRelativeSteps(int32_t steps, uint32_t stepsPerSec);

/* MOTOR CONTROL *** END *************************************************************************/



extern void checkStateIOPins();
extern void setupIO();



/* TODO: UNDEFINE TEST_STEP_DRIVER FOR PRODUCTION */ 
#define TEST_STEP_DRIVER
#ifdef TEST_STEP_DRIVER
extern void motorBackNForth();
#endif /* TEST_STEP_DRIVER */



#endif /* X_IO_H */