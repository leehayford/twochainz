#include "x_io.h"


/* INTERRUPTS *************************************************************************************/
hw_timer_t *debounceTimer = NULL;
enum itrCheckMap { CHK_ESTOP = 0, CHK_DOOR, CHK_ARM, CHK_ANVIL };
int itrCheck[3]; 
int itrNow;

void debounce(int itrPin) { itrCheck[itrPin] = millis() + ITR_DEBOUNCE_ALARM_INC_mSEC; }
void IRAM_ATTR eStopInturrupt() { debounce(CHK_ESTOP); }
void IRAM_ATTR doorInturrupt() { debounce(CHK_DOOR); }
void IRAM_ATTR armInturrupt() { debounce(CHK_ARM); }
void IRAM_ATTR anvilInturrupt() { debounce(CHK_ANVIL); }

void itrHandleEStopState() { sta.doorClosed = !digitalRead(PIN_ITR_DOOR); }
void itrHandleDoorState() { sta.doorClosed = !digitalRead(PIN_ITR_DOOR); }
void itrHandleArmState() { sta.armContact = !digitalRead(PIN_ITR_ARM); }
void itrHandleAnvilState() { sta.anvilContact = !digitalRead(PIN_ITR_ANVIL); }
void IRAM_ATTR onDebounceTimer() {
    itrNow = millis();

    if (itrCheck[CHK_DOOR] && itrCheck[CHK_DOOR] <= itrNow) { 
        itrCheck[CHK_DOOR] = 0;
        itrHandleDoorState(); 
    }
    
    if (itrCheck[CHK_ARM] && itrCheck[CHK_ARM] <= itrNow) { 
        itrCheck[CHK_ARM] = 0;
        itrHandleArmState();
    }
    
    if (itrCheck[CHK_ANVIL] && itrCheck[CHK_ANVIL] <= itrNow) { 
        itrCheck[CHK_ANVIL] = 0;
        itrHandleAnvilState(); 
    }

}

void setupDebounceTimer() {
    
    debounceTimer = timerBegin(
        ITR_DEBOUNCE_TIMER, 
        ITR_DEBOUNCE_PRESCALE, 
        ITR_DEBOUNCE_COUNT_UP
    );
    
    timerAttachInterrupt(
        debounceTimer, 
        onDebounceTimer, 
        ITR_DEBOUNCE_EDGE 
    );

    timerAlarmWrite(
        debounceTimer, 
        ITR_DEBOUNCE_TIMER_PERIOD_uSEC, 
        ITR_DEBOUNCE_AUTORUN
    );

    timerAlarmEnable(debounceTimer);
}

/* INTERRUPTS *** END ****************************************************************************/



/* RELAY CONTROLS *******************************************************************************/

/* RELAY CONTROLS *** END **********************************************************************/



/* MOTOR CONTROLS *****************************************************************************/


/* MOTOR CONTROLS *** END ********************************************************************/



void setupIO() {

    /* INTERRUPTS *************************************************************************************/

    pinMode(PIN_ITR_DOOR, INPUT_PULLUP);
    pinMode(PIN_ITR_ARM, INPUT_PULLUP);
    pinMode(PIN_ITR_ANVIL, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(PIN_ITR_DOOR), doorInturrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ITR_ARM), doorInturrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ITR_ANVIL), doorInturrupt, CHANGE);

    setupDebounceTimer();

    /* INTERRUPTS *** END ****************************************************************************/



    /* RELAY CONTROLS *******************************************************************************/

    /* RELAY CONTROLS *** END **********************************************************************/



    /* MOTOR CONTROLS *****************************************************************************/
    pinMode(PIN_OUT_MOT_STEP, OUTPUT);
    pinMode(PIN_OUT_MOT_DIR, OUTPUT);
    pinMode(PIN_OUT_MOT_EN, OUTPUT);
    pinMode(PIN_OUT_MOT_MS1, OUTPUT);
    pinMode(PIN_OUT_MOT_MS2, OUTPUT);
    pinMode(PIN_OUT_MOT_MS3, OUTPUT);

    digitalWrite(PIN_OUT_MOT_DIR, MOT_DIR_DOWN);
    digitalWrite(PIN_OUT_MOT_EN, MOT_DISABLED);

    // stepper.connectToPins(PIN_OUT_MOT_STEP, PIN_OUT_MOT_DIR);
    // stepper.setSpeedInStepsPerSecond(MOT_STEPS_PER_SEC);
    // stepper.setAccelerationInStepsPerSecondPerSecond(MOT_ACCEL_STEPS_PER_SEC);
    // stepper.setDecelerationInStepsPerSecondPerSecond(MOT_ACCEL_STEPS_PER_SEC);
    // stepper.startAsService(MOT_SERVICE_CORE);

    /* MOTOR CONTROLS *** END ********************************************************************/
}
