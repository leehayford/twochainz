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

void itrClearCheck(int chk, int pin, bool *pinState) { 
    if (itrCheck[chk] > 0 && itrCheck[chk] <= itrNow && *pinState != digitalRead(pin) ) {
            *pinState = digitalRead(pin); 
            itrCheck[chk] = 0; 
            // checkAlarmsFlag = 1;
    }
}
void IRAM_ATTR onDebounceTimer() {
    itrNow = millis();
    itrClearCheck(CHK_ESTOP, PIN_ITR_ESTOP, &sta.emergencyStop);
    itrClearCheck(CHK_DOOR, PIN_ITR_DOOR, &sta.doorClosed);
    itrClearCheck(CHK_FIST, PIN_ITR_FIST, &sta.fistContact);
    itrClearCheck(CHK_ANVIL, PIN_ITR_ANVIL, &sta.anvilContact);
    itrClearCheck(CHK_TOP, PIN_ITR_TOP, &sta.topContact);
    itrClearCheck(CHK_PRESSURE, PIN_ITR_PRESSURE, &sta.pressureContact);
}

void debounce(int chk) { 
    if (itrCheck[chk] == 0) {
        itrCheck[chk] = millis() + ITR_DEBOUNCE_ALARM_INC_mSEC; 
    }
}
void IRAM_ATTR eStopInturrupt() { debounce(CHK_ESTOP); }
void IRAM_ATTR doorInturrupt() { debounce(CHK_DOOR); }
void IRAM_ATTR fistInturrupt() { debounce(CHK_FIST); }
void IRAM_ATTR anvilInturrupt() { debounce(CHK_ANVIL); }
void IRAM_ATTR topInturrupt() { debounce(CHK_TOP); }
void IRAM_ATTR pressurelInturrupt() { debounce(CHK_PRESSURE); }

void setupInterrupts() {
    pinMode(PIN_ITR_ESTOP, INPUT_PULLUP);
    pinMode(PIN_ITR_DOOR, INPUT_PULLUP);
    pinMode(PIN_ITR_FIST, INPUT_PULLUP);
    pinMode(PIN_ITR_ANVIL, INPUT_PULLUP);
    pinMode(PIN_ITR_TOP, INPUT_PULLUP);
    pinMode(PIN_ITR_PRESSURE, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(PIN_ITR_ESTOP), eStopInturrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ITR_DOOR), doorInturrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ITR_FIST), fistInturrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ITR_ANVIL), anvilInturrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ITR_TOP), topInturrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ITR_PRESSURE), pressurelInturrupt, CHANGE);

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

/* INTERRUPTS *** END ********************************************************************************/



/* RELAY CONTROL ************************************************************************************/

void brakeOn() { /* TODO: OK TO APPLY BREAK */
    digitalWrite(PIN_OUT_BRAKE, BREAK_ON); 
}
void brakeOff() { /* TODO: OK TO RELESE BREAK */
   digitalWrite(PIN_OUT_BRAKE, BREAK_OFF);
}

void magnetOn() { /* TODO: OK TO GRAB HAMMER */
   digitalWrite(PIN_OUT_MAGNET, MAGNET_ON);
}
void magnetOff() { /* TODO: OK TO DROP HAMMER */
    digitalWrite(PIN_OUT_MAGNET, MAGNET_OFF); 
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

void setupMotor() {

    pinMode(PIN_OUT_MOT_STEP, OUTPUT);
    pinMode(PIN_OUT_MOT_DIR, OUTPUT);
    pinMode(PIN_OUT_MOT_EN, OUTPUT);

    motorStop();
    
    motor.connectToPins(PIN_OUT_MOT_STEP, PIN_OUT_MOT_DIR);
    setMotorSpeed(MOT_STEPS_PER_SEC_LOW);
    motor.startAsService(MOT_SERVICE_CORE);
}

void emergencyStop() { /* TODO: ORDER TO STOP DOING STUFF */ }
void moveToHome() { /* TODO: OK TO MOVE DOWN */ }
void moveToHammer() { /* TODO: OK TO MOVE DOWN */ }
void moveToSwingHeight() { /* TODO: OK TO MOVE UP */ }

/* MOTOR CONTROL *** END *************************************************************************/


/* TODO: UNDEFINE TEST_STEP_DRIVER FOR PRODUCTION */ 
#ifdef TEST_STEP_DRIVER
int steps = 16000;
void motorBackNForth() {
    if(sta.cyclesCompleted <= cfg.cycles) {
        if (motor.getDistanceToTargetSigned() == 0) {
            if(sta.cyclesCompleted == 0) {
                steps = ( cfg.height / FIST_INCH_PER_REV ) * MOT_STEP_PER_REV;
            } else {
                steps *= -1;
            }
            motorEnable();
            setMotorSpeed(MOT_STEPS_PER_SEC_HIGH);
            delay(1000);
            if (steps < 0) {
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

void setupIO() {
    setupInterrupts();
    setupRelayControl();
    setupMotor();
}
