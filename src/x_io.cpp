#include "x_io.h"


/* INTERRUPTS *****************************************************************************************/

// uint32_t g_ui32InterruptFlag = 0;

ITRPin itrpEStop(PIN_ITR_ESTOP, &g_state.eStop, ITR_PIN_ACTIVE_LOW);
ITRPin itrpDoor(PIN_ITR_DOOR, &g_state.doorOpen, ITR_PIN_ACTIVE_HIGH);
ITRPin itrpFist(PIN_ITR_FIST, &g_state.fistLimit, ITR_PIN_ACTIVE_LOW);
ITRPin itrpAnvil(PIN_ITR_ANVIL, &g_state.anvilLimit, ITR_PIN_ACTIVE_LOW);
ITRPin itrpHome(PIN_ITR_HOME, &g_state.homeLimit, ITR_PIN_ACTIVE_LOW);
ITRPin itrpTop(PIN_ITR_TOP, &g_state.topLimit, ITR_PIN_ACTIVE_LOW);
ITRPin itrpPressure(PIN_ITR_PRESSURE, &g_state.pressure, ITR_PIN_ACTIVE_HIGH);

static void setITRPinDebounce(ITRPin *itrp) {
    if( itrp->checkTime == 0 )                                  /* This is a new event */
        itrp->checkTime = millis() + itrp->debouncePeriod;      // Set the debounce alarm time
}
void IRAM_ATTR isrEStop() { setITRPinDebounce(&itrpEStop); }
void IRAM_ATTR isrDoor() { setITRPinDebounce(&itrpDoor); }
void IRAM_ATTR isrFist() { setITRPinDebounce(&itrpFist); }
void IRAM_ATTR isrAnvil() { setITRPinDebounce(&itrpAnvil); }
void IRAM_ATTR isrHome() { setITRPinDebounce(&itrpHome); }
void IRAM_ATTR isrTop() { setITRPinDebounce(&itrpTop); }
void IRAM_ATTR isrPressure() { setITRPinDebounce(&itrpPressure); }


void setupInterrupts() {

    itrpEStop.setupPin(isrEStop);
    itrpDoor.setupPin(isrDoor);
    itrpFist.setupPin(isrFist);
    itrpAnvil.setupPin(isrAnvil);
    itrpHome.setupPin(isrHome);
    itrpTop.setupPin(isrTop);
    itrpPressure.setupPin(isrPressure);

}

/* INTERRUPTS *** END ********************************************************************************/


/* TIMERS *********************************************************************************************/

/* INTERRUPT PIN DEBOUNCE CHECK TIMER */
uint32_t m_ui32NowMillis;
void IRAM_ATTR isrDebounceTimer() {;

    m_ui32NowMillis = millis();

    itrpEStop.itrCheck(m_ui32NowMillis);
    itrpDoor.itrCheck(m_ui32NowMillis);
    itrpFist.itrCheck(m_ui32NowMillis);
    itrpAnvil.itrCheck(m_ui32NowMillis);
    itrpHome.itrCheck(m_ui32NowMillis);
    itrpTop.itrCheck(m_ui32NowMillis);
    itrpPressure.itrCheck(m_ui32NowMillis);

}

hw_timer_t *tmrITRDebounce = NULL; 
void setupITRDebounceTimer() {

    tmrITRDebounce = timerBegin(
        ITR_DEBOUNCE_TIMER, 
        ITR_DEBOUNCE_TIMER_PRESCALE, 
        ITR_DEBOUNCE_TIMER_COUNT_UP
    );
    
    timerAttachInterrupt(
        tmrITRDebounce, 
        isrDebounceTimer, 
        ITR_DEBOUNCE_TIMER_EDGE 
    );

    timerAlarmWrite(
        tmrITRDebounce, 
        ITR_DEBOUNCE_TIMER_PERIOD_uSEC, 
        ITR_DEBOUNCE_TIMER_AUTORUN
    );

    timerAlarmEnable(tmrITRDebounce);
}

void changeITRDebounceTimerPeriod() {

    timerAlarmWrite(
        tmrITRDebounce, 
        g_admin.ioTmrITRDeb_uSec, 
        ITR_DEBOUNCE_TIMER_AUTORUN
    );
}

/* Hammertime stuff */
hw_timer_t *tmrHammerStrike = NULL;
void setupHammerTimer() {
    tmrHammerStrike = timerBegin(
        OPS_HAMMER_TIMER,
        OPS_HAMMER_TIMER_PRESCALE,
        OPS_HAMMER_TIMER_COUNT_UP
    );

    timerAttachInterrupt(
        tmrHammerStrike,
        [](){ g_state.hammerTimeout = true; },
        OPS_HAMMER_TIMER_EDGE
    );

    timerAlarmWrite(
        tmrHammerStrike,
        OPS_HAMMER_TIMER_STRIKE_PERIOD_uSEC,
        OPS_HAMMER_TIMER_RUN_ONCE
    );
    
}

void startHammerTimer() {
    g_state.hammerTimeout = false;          // Clear the flag
    timerRestart(tmrHammerStrike);             // Necessary for calls subsequent to the first call
    timerAlarmEnable(tmrHammerStrike);         // Hammertime is upon us
}

void changeHammerTimeroutPeriod() {
    timerAlarmWrite(
        tmrHammerStrike,
        g_admin.opsTmrHammer_uSec,
        OPS_HAMMER_TIMER_RUN_ONCE
    );
}

void setupTimers() {
    setupITRDebounceTimer();
    setupHammerTimer();
}

/* TIMERS *** END *************************************************************************************/



/* DIGITAL OUT **************************************************************************************/
DOUTPin doutBrake(PIN_OUT_BRAKE, &g_state.breakOn, DOUT_PIN_ACTIVE_LOW);
void brakeOn() { doutBrake.enable(); }      /* TODO: ERROR CHECKING */
void brakeOff() { doutBrake.disable(); }    /* TODO: ERROR CHECKING */

DOUTPin doutMagnet(PIN_OUT_MAGNET, &g_state.magnetOn, DOUT_PIN_ACTIVE_HIGH);
void magnetOn() { doutMagnet.enable(); }    /* TODO: ERROR CHECKING */
void magnetOff() { doutMagnet.disable(); }  /* TODO: ERROR CHECKING */

DOUTPin doutMotDir(PIN_OUT_MOT_DIR, nullptr, NULL, DOUT_PIN_ACTIVE_LOW);

DOUTPin doutMotStep(PIN_OUT_MOT_STEP, nullptr, NULL, DOUT_PIN_ACTIVE_LOW);

void setupDigitalOutputs() {
    doutBrake.setupPin();
    doutMagnet.setupPin();
    doutMotDir.setupPin();
    doutMotStep.setupPin();
}

/* DIGITAL OUT *** END *****************************************************************************/



/* MOTOR CONTROL ***********************************************************************************/
ESP_FlexyStepper m_motor;

Alert ALERT_MOT_POS_FIX_LOST("motor position fix has been lost");
Alert* motorGetPosition() {

    g_state.motorSteps = m_motor.getCurrentPositionInSteps();

    g_state.currentHeight = 
        ((float)g_state.motorSteps / g_admin.motStepsRev) 
        * g_admin.motInchRev;

    // if( !g_ops.diagnosticMode                               /* We're not currently diagnosing something */
    // &&  (   g_state.motorSteps < -3                          /* We're lost */
    //     ||  g_state.motorSteps > FIST_HEIGHT_MAX_STEP       /* We're lost */
    //     )
    // )  { 

    //     return &ALERT_MOT_POS_FIX_LOST;                     // Tell everyone we're lost 
    // }
    return nullptr;
}

Alert* motorSetPositionAsZero() {
    m_motor.setCurrentPositionInSteps(0);
    return motorGetPosition();
}

Alert ALERT_MOT_SPEED_TOO_HIGH("motor target speed is too high", WARNING);
Alert ALERT_MOT_SPEED_TOO_LOW("motor target speed is too low", WARNING);
Alert* motorSetSpeed(uint32_t stepsPerSec) {

    Alert* warn = nullptr;

    if( stepsPerSec > g_admin.motHzHigh                 /* We've been instructed poorly */
    ) { 
        stepsPerSec = g_admin.motHzHigh;                // We know better
        warn = &ALERT_MOT_SPEED_TOO_HIGH;               // We level a warning
    }

    else 
    if( stepsPerSec < 1                                 /* We've been instructed poorly */
    ) { 
        stepsPerSec = g_admin.motHzLow;                 // We know better
        warn = &ALERT_MOT_SPEED_TOO_LOW;                // We level a warning
    }

    m_motor.setSpeedInStepsPerSecond(stepsPerSec);
    m_motor.setAccelerationInStepsPerSecondPerSecond(g_admin.motAccel);
    m_motor.setDecelerationInStepsPerSecondPerSecond(g_admin.motDecel);

    return warn;
}

void motorSetCourse(int32_t steps) { 
    m_motor.setTargetPositionRelativeInSteps(steps); 
}

void motorStop() { 
    // motorSetCourse(0); 
    m_motor.emergencyStop();
}

bool motorTargetReached() { 
    return (m_motor.getDistanceToTargetSigned() == 0); 
}

void setupMotor() {
    m_motor.connectToPins(doutMotStep.pin, doutMotDir.pin);
    m_motor.startAsService(MOT_SERVICE_CORE);
    motorStop();
}


/* MOTOR CONTROL *** END *************************************************************************/



void checkStateIOPins() {

    itrpEStop.checkPin();
    itrpDoor.checkPin();
    itrpFist.checkPin();
    itrpAnvil.checkPin();
    itrpHome.checkPin();
    itrpTop.checkPin();
    itrpPressure.checkPin();

    doutBrake.checkPin();
    doutMagnet.checkPin();
}

void setupIO() {

    setupTimers();

    setupInterrupts();

    setupDigitalOutputs();
    
    setupMotor();
    
    checkStateIOPins();
}

