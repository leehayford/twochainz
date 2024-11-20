#include "x_io.h"


/* INTERRUPTS *****************************************************************************************/
hw_timer_t *debounceTimer = NULL;
enum itrCheckMap { 
    CHK_ESTOP = 0, 
    CHK_DOOR, 
    CHK_FIST, 
    CHK_ANVIL,
    CHK_TOP,
    CHK_PRESSURE 
};
int itrCheck[6]; 
int itrNow;

void debounce(int itrPin) { 
    if (itrCheck[itrPin] == 0) { 
        itrCheck[itrPin] = millis() + ITR_DEBOUNCE_ALARM_INC_mSEC; 
    } 
}
void IRAM_ATTR eStopInturrupt() { debounce(CHK_ESTOP); }
void IRAM_ATTR doorInturrupt() { debounce(CHK_DOOR); }
void IRAM_ATTR fistInturrupt() { debounce(CHK_FIST); }
void IRAM_ATTR anvilInturrupt() { debounce(CHK_ANVIL); }
void IRAM_ATTR topInturrupt() { debounce(CHK_TOP); }
void IRAM_ATTR pressurelInturrupt() { debounce(CHK_PRESSURE); }

void itrClearCheck(int itrPin) {
        itrCheck[itrPin] = 0;
        sta.send = true;
}
void itrHandleEStopState() { sta.emergencyStop = !digitalRead(PIN_ITR_ESTOP); } // void itrHandleDoorState() { sta.setDoorClosed(!digitalRead(PIN_ITR_DOOR)); }
void itrHandleDoorState() { sta.doorClosed = !digitalRead(PIN_ITR_DOOR); }
void itrHandleFistState() { sta.fistContact = !digitalRead(PIN_ITR_FIST); }
void itrHandleAnvilState() { sta.anvilContact = !digitalRead(PIN_ITR_ANVIL); }
void itrHandleTopState() { sta.topContact = !digitalRead(PIN_ITR_TOP); }
void itrHandlePressureState() { sta.pressureContact = !digitalRead(PIN_ITR_PRESSURE); }

void IRAM_ATTR onDebounceTimer() {
    itrNow = millis();

    if (itrCheck[CHK_ESTOP] > 0 && itrCheck[CHK_ESTOP] <= itrNow) { 
        itrHandleEStopState(); 
        itrClearCheck(CHK_ESTOP);
    }

    if (itrCheck[CHK_DOOR] > 0 && itrCheck[CHK_DOOR] <= itrNow) { 
        itrHandleDoorState(); 
        itrClearCheck(CHK_DOOR);
    }
    
    if (itrCheck[CHK_FIST] > 0 && itrCheck[CHK_FIST] <= itrNow) { 
        itrHandleFistState();
        itrClearCheck(CHK_FIST);
    }
    
    if (itrCheck[CHK_ANVIL] > 0 && itrCheck[CHK_ANVIL] <= itrNow) { 
        itrHandleAnvilState(); 
        itrClearCheck(CHK_ANVIL);
    }
    
    if (itrCheck[CHK_TOP] > 0 && itrCheck[CHK_TOP] <= itrNow) { 
        itrHandleTopState(); 
        itrClearCheck(CHK_TOP);
    }
    
    if (itrCheck[CHK_PRESSURE] > 0 && itrCheck[CHK_PRESSURE] <= itrNow) { 
        itrHandlePressureState(); 
        itrClearCheck(CHK_PRESSURE);
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
void setupInterrupts() {
    pinMode(PIN_ITR_ESTOP, INPUT_PULLUP);
    pinMode(PIN_ITR_DOOR, INPUT_PULLUP);
    pinMode(PIN_ITR_FIST, INPUT_PULLUP);
    pinMode(PIN_ITR_ANVIL, INPUT_PULLUP);
    pinMode(PIN_ITR_TOP, INPUT_PULLUP);
    pinMode(PIN_ITR_PRESSURE, INPUT_PULLUP);

    // attachInterrupt(PIN_ITR_ESTOP, eStopInturrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ITR_ESTOP), eStopInturrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ITR_DOOR), doorInturrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ITR_FIST), fistInturrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ITR_ANVIL), anvilInturrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ITR_TOP), topInturrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ITR_PRESSURE), pressurelInturrupt, CHANGE);

    setupDebounceTimer();
}

/* INTERRUPTS *** END ********************************************************************************/



/* RELAY CONTROL ************************************************************************************/

void brakeOn() { digitalWrite(PIN_OUT_BRAKE, BREAK_ON); }
void brakeOff() { /* TODO: OK TO RELESE BREAK:
        
        1. WE ARE DROPPING THE HAMMER ON PURPOSE
            sta.doorClosed
            &&
            sta.fistContact 
            && 
            sta.magnetOn 
            &&
            sta.currentHeight == cfg.height

    */
   digitalWrite(PIN_OUT_BRAKE, BREAK_OFF);
}

void magnetOff() { digitalWrite(PIN_OUT_MAGNET, MAGNET_OFF); }
void magnetOn() { /* TODO: OK TO ENABLE MAGNET:
        
        1. sta.fistContact && sta.brakeOn 

    */
   digitalWrite(PIN_OUT_MAGNET, MAGNET_ON);
}

void setupRelayControl() {
    pinMode(PIN_OUT_BRAKE, OUTPUT);
    brakeOn();

    pinMode(PIN_OUT_MAGNET, OUTPUT);
    magnetOff();
}

/* RELAY CONTROL *** END ***************************************************************************/



/* MOTOR CONTROL ***********************************************************************************/
ESP_FlexyStepper motor;

void setMotorSpeed(int stepsPerSec) {
    if (stepsPerSec > MOT_STEPS_PER_SEC_HIGH) { stepsPerSec = MOT_STEPS_PER_SEC_HIGH; }
    motor.setSpeedInStepsPerSecond(stepsPerSec);
    motor.setAccelerationInStepsPerSecondPerSecond(MOT_STEPS_PER_SEC_ACCEL);
    motor.setDecelerationInStepsPerSecondPerSecond(MOT_STEPS_PER_SEC_ACCEL);
}

void motorStop() { digitalWrite(PIN_OUT_MOT_EN, MOT_DISABLED); }
void motorEnable() { digitalWrite(PIN_OUT_MOT_EN, MOT_ENABLED); }

void moveToHome() {
    /* If we asre not holding the hammer, 
        go get it */


}
void moveToHammer() {}
void moveToSwingHeight() {}
void emergencyStop() {
    brakeOn();
    magnetOff();
    motorStop();
}

void setupMotor() {

    pinMode(PIN_OUT_MOT_STEP, OUTPUT);
    pinMode(PIN_OUT_MOT_DIR, OUTPUT);
    pinMode(PIN_OUT_MOT_EN, OUTPUT);

    motorStop();
    
    motor.connectToPins(PIN_OUT_MOT_STEP, PIN_OUT_MOT_DIR);
    setMotorSpeed(MOT_STEPS_PER_SEC_LOW);
    motor.startAsService(MOT_SERVICE_CORE);
}

/* TODO: UNDEFINE TEST_STEP_DRIVER FOR PRODUCTION */ 
#ifdef TEST_STEP_DRIVER
int steps = 16000;
void motorBackNForth() {
    if(sta.cyclesCompleted < cfg.cycles) {
        if(sta.cyclesCompleted == 0) {
            steps = ( cfg.height / FIST_INCH_PER_REV ) * MOT_STEP_PER_REV;
        }
        if (motor.getDistanceToTargetSigned() == 0) {
            steps *= -1;
            motorEnable();
            setMotorSpeed(MOT_STEPS_PER_SEC_HIGH);
            delay(1000);
            if (steps > 0) {
                magnetOn();
                brakeOff();
            } else {
                magnetOff();
                brakeOn();
                sta.cyclesCompleted++;
            }
            motor.setTargetPositionRelativeInSteps(steps);
        }
    }
}
#endif /* TEST_STEP_DRIVER */

/* MOTOR CONTROL *** END *************************************************************************/


void setupIO() {
    setupInterrupts();
    setupRelayControl();
    setupMotor();
}
