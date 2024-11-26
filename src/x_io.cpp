#include "x_io.h"
 
/* INTERRUPTS *****************************************************************************************/
hw_timer_t *m_phwTimerDebounce = NULL;
uint32_t m_aui32ItrCheck[ITR_PIN_COUNT]; 
uint32_t m_ui32NowMillis;
uint32_t g_ui32InterruptFlag = 0;
bool m_bPrevPinState = false;
void itrClearCheck(eItrCheckMap_t eChk, uint8_t pin, bool *pbPinState, bool bInvert = false) { 

    if ( ( m_aui32ItrCheck[eChk] == 0 ) || ( m_aui32ItrCheck[eChk] > m_ui32NowMillis ) ) 
        return;

    else {
        m_bPrevPinState = *pbPinState;

        *pbPinState = ( bInvert ? !(bool)digitalRead(pin) : (bool)digitalRead(pin) );
        m_aui32ItrCheck[eChk] = 0; 

        if ( m_bPrevPinState != *pbPinState )
            g_ui32InterruptFlag = 1;
    }
}

void IRAM_ATTR isrDebounceTimer() {
    // timerAlarmDisable(m_phwTimerDebounce);

    m_ui32NowMillis = millis();
    itrClearCheck(CHK_ESTOP, PIN_ITR_ESTOP, &g_state.eStop, ITR_INVERT_PIN_STATE);
    itrClearCheck(CHK_DOOR, PIN_ITR_DOOR, &g_state.doorOpen);
    itrClearCheck(CHK_FIST, PIN_ITR_FIST, &g_state.fistLimit, ITR_INVERT_PIN_STATE);
    itrClearCheck(CHK_ANVIL, PIN_ITR_ANVIL, &g_state.anvilLimit, ITR_INVERT_PIN_STATE);
    itrClearCheck(CHK_TOP, PIN_ITR_TOP, &g_state.topLimit, ITR_INVERT_PIN_STATE);
    itrClearCheck(CHK_PRESSURE, PIN_ITR_PRESSURE, &g_state.pressure, ITR_INVERT_PIN_STATE);
    
    // timerAlarmEnable(m_phwTimerDebounce);
}

void debounce(eItrCheckMap_t eChk) { 
    if (m_aui32ItrCheck[eChk] == 0) {
        m_aui32ItrCheck[eChk] = millis() + ITR_DEBOUNCE_ALARM_INC_mSEC; 
    }
}

void IRAM_ATTR isrEStop() { debounce(CHK_ESTOP); }
void IRAM_ATTR isrDoor() { debounce(CHK_DOOR); }
void IRAM_ATTR isrFistLimit() { debounce(CHK_FIST); }
void IRAM_ATTR isrAnvilLimit() { debounce(CHK_ANVIL); }
void IRAM_ATTR isrTopLimit() { debounce(CHK_TOP); }
void IRAM_ATTR isrPressure() { debounce(CHK_PRESSURE); }

void setupInterrupts() {
    pinMode(PIN_ITR_ESTOP, INPUT_PULLUP);
    pinMode(PIN_ITR_DOOR, INPUT_PULLUP);
    pinMode(PIN_ITR_FIST, INPUT_PULLUP);
    pinMode(PIN_ITR_ANVIL, INPUT_PULLUP);
    pinMode(PIN_ITR_TOP, INPUT_PULLUP);
    pinMode(PIN_ITR_PRESSURE, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(PIN_ITR_ESTOP), isrEStop, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ITR_DOOR), isrDoor, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ITR_FIST), isrFistLimit, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ITR_ANVIL), isrAnvilLimit, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ITR_TOP), isrTopLimit, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ITR_PRESSURE), isrPressure, CHANGE);

    m_phwTimerDebounce = timerBegin(
        ITR_DEBOUNCE_TIMER, 
        ITR_DEBOUNCE_PRESCALE, 
        ITR_DEBOUNCE_COUNT_UP
    );
    
    timerAttachInterrupt(
        m_phwTimerDebounce, 
        isrDebounceTimer, 
        ITR_DEBOUNCE_EDGE 
    );

    timerAlarmWrite(
        m_phwTimerDebounce, 
        ITR_DEBOUNCE_TIMER_PERIOD_uSEC, 
        ITR_DEBOUNCE_AUTORUN
    );

    timerAlarmEnable(m_phwTimerDebounce);
}

/* INTERRUPTS *** END ********************************************************************************/



/* RELAY CONTROL ************************************************************************************/

void switchBrake(int setting) {
    digitalWrite(PIN_OUT_BRAKE, setting); 
    g_state.breakOn = (bool)digitalRead(PIN_OUT_BRAKE);
}
void brakeOn() { switchBrake(BREAK_ON); }
void brakeOff() { switchBrake(BREAK_OFF); }

void switchMagnet(int setting) {
    digitalWrite(PIN_OUT_MAGNET, setting); 
    g_state.magnetOn = (bool)digitalRead(PIN_OUT_MAGNET);
}
void magnetOn() { switchMagnet(MAGNET_ON); }
void magnetOff() { switchMagnet(MAGNET_OFF); }

void setupRelayControl() {
    pinMode(PIN_OUT_BRAKE, OUTPUT);
    brakeOn();

    pinMode(PIN_OUT_MAGNET, OUTPUT);
    magnetOff();
}

/* RELAY CONTROL *** END ***************************************************************************/



/* MOTOR CONTROL ***********************************************************************************/
ESP_FlexyStepper m_motor;

void setMotorSpeed(int stepsPerSec) {
    if (stepsPerSec > MOT_STEPS_PER_SEC_HIGH) { stepsPerSec = MOT_STEPS_PER_SEC_HIGH; }
    m_motor.setSpeedInStepsPerSecond(stepsPerSec);
    m_motor.setAccelerationInStepsPerSecondPerSecond(MOT_STEPS_PER_SEC_ACCEL);
    m_motor.setDecelerationInStepsPerSecondPerSecond(MOT_STEPS_PER_SEC_ACCEL);
}

void motorOn() { 
    digitalWrite(PIN_OUT_MOT_EN, MOT_ENABLED); 
    g_state.motorOn = true;
}
void motorOff() { 
    Serial.printf("\nmotorOff()");
    digitalWrite(PIN_OUT_MOT_EN, MOT_DISABLED); 
    g_state.motorOn = false;
}

void setupMotor() {

    pinMode(PIN_OUT_MOT_STEP, OUTPUT);
    pinMode(PIN_OUT_MOT_DIR, OUTPUT);
    pinMode(PIN_OUT_MOT_EN, OUTPUT);

    motorOff();
    
    m_motor.connectToPins(PIN_OUT_MOT_STEP, PIN_OUT_MOT_DIR);
    setMotorSpeed(MOT_STEPS_PER_SEC_LOW);
    m_motor.startAsService(MOT_SERVICE_CORE);
}

// void emergencyStop() { /* TODO: ORDER TO STOP DOING STUFF */ }
// void moveToHome() { /* TODO: OK TO MOVE DOWN */ }
// void moveToHammer() { /* TODO: OK TO MOVE DOWN */ }
// void moveToSwingHeight() { /* TODO: OK TO MOVE UP */ }

/* MOTOR CONTROL *** END *************************************************************************/


/* TODO: UNDEFINE TEST_STEP_DRIVER FOR PRODUCTION */ 
#ifdef TEST_STEP_DRIVER
int32_t m_steps = 16000;
void motorBackNForth() {
    if(g_state.cyclesCompleted <= g_config.cycles) {
        if (m_motor.getDistanceToTargetSigned() == 0) {
            if(g_state.cyclesCompleted == 0) {
                m_steps = ( g_config.height / FIST_INCH_PER_REV ) * MOT_STEP_PER_REV;
            } else {
                m_steps *= -1;
            }
            motorOn();
            setMotorSpeed(MOT_STEPS_PER_SEC_HIGH);
            delay(1000);
            if (m_steps < 0) {
                magnetOn();
                brakeOff();
            } else {
                magnetOff();
                brakeOn();
                g_state.cyclesCompleted++;
            }
            m_motor.setTargetPositionRelativeInSteps(m_steps);
        }
    }
}
#endif /* TEST_STEP_DRIVER */

void setupIO() {
    setupInterrupts();
    setupRelayControl();
    setupMotor();
}
