#include "x_io.h"
 
/* INTERRUPTS *****************************************************************************************/

uint32_t g_ui32InterruptFlag = 0;

ITRPin itrpEStop(PIN_ITR_ESTOP, &g_state.eStop, ITR_PIN_ACTIVE_LOW);
ITRPin itrpDoor(PIN_ITR_DOOR, &g_state.doorOpen, ITR_PIN_ACTIVE_HIGH);
ITRPin itrpFist(PIN_ITR_FIST, &g_state.fistLimit, ITR_PIN_ACTIVE_LOW);
ITRPin itrpAnvil(PIN_ITR_ANVIL, &g_state.anvilLimit, ITR_PIN_ACTIVE_LOW);
ITRPin itrpHome(PIN_ITR_HOME, &g_state.homeLimit, ITR_PIN_ACTIVE_LOW);
ITRPin itrpTop(PIN_ITR_TOP, &g_state.topLimit, ITR_PIN_ACTIVE_LOW);
ITRPin itrpPressure(PIN_ITR_PRESSURE, &g_state.pressure, ITR_PIN_ACTIVE_LOW);

static void itrDebounce(ITRPin *itrp) {
    if( itrp->checkTime == 0 )                               /* This is a new event */
        itrp->checkTime = millis() + itrp->debouncePeriod;    // Set the debounce alarm time
}
void IRAM_ATTR isrEStop() { itrDebounce(&itrpEStop); }
void IRAM_ATTR isrDoor() { itrDebounce(&itrpDoor); }
void IRAM_ATTR isrFist() { itrDebounce(&itrpFist); }
void IRAM_ATTR isrAnvil() { itrDebounce(&itrpAnvil); }
void IRAM_ATTR isrHome() { itrDebounce(&itrpHome); }
void IRAM_ATTR isrTop() { itrDebounce(&itrpTop); }
void IRAM_ATTR isrPressure() { itrDebounce(&itrpPressure); }


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

hw_timer_t *m_phwTimerDebounce = NULL; 
void setupInterrupts() {

    itrpEStop.setupPin(isrEStop);
    itrpDoor.setupPin(isrDoor);
    itrpFist.setupPin(isrFist);
    itrpAnvil.setupPin(isrAnvil);
    itrpHome.setupPin(isrHome);
    itrpTop.setupPin(isrTop);
    itrpPressure.setupPin(isrPressure);

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


/* DIGITAL OUT **************************************************************************************/
DOUTPin doutBrake(PIN_OUT_BRAKE, &g_state.breakOn, DOUT_PIN_ACTIVE_LOW);
void brakeOn() { doutBrake.enable(); } /* TODO: ERROR CHECKING */
void brakeOff() { doutBrake.disable(); } /* TODO: ERROR CHECKING */

DOUTPin doutMagnet(PIN_OUT_MAGNET, &g_state.magnetOn, DOUT_PIN_ACTIVE_HIGH);
void magnetOn() { doutMagnet.enable(); } /* TODO: ERROR CHECKING */
void magnetOff() { doutMagnet.disable(); } /* TODO: ERROR CHECKING */

DOUTPin doutMotor(PIN_OUT_MOT_EN, &g_state.motorOn, DOUT_PIN_ACTIVE_LOW);
void motorOn() { doutMotor.enable(); } /* TODO: ERROR CHECKING */
void motorOff() { doutMotor.disable(); } /* TODO: ERROR CHECKING */

DOUTPin doutMotDir(PIN_OUT_MOT_DIR, nullptr, NULL, DOUT_SANS_STATE);

DOUTPin doutMotStep(PIN_OUT_MOT_STEP, nullptr, NULL, DOUT_SANS_STATE);


void setupDigitalOutputs() {

    doutBrake.setupPin();
    doutMagnet.setupPin();
    doutMotor.setupPin();
    doutMotDir.setupPin();
    doutMotStep.setupPin();

}

/* DIGITAL OUT *** END *****************************************************************************/



/* MOTOR CONTROL ***********************************************************************************/
ESP_FlexyStepper m_motor;

Error ERR_MOT_POS_FIX_LOST("motor position fix has been lost");
Error* motorGetPosition() {

    g_state.motorSteps = m_motor.getCurrentPositionInSteps();

    g_state.currentHeight = 
        ((float)g_state.motorSteps / MOT_STEP_PER_REV) 
        * FIST_INCH_PER_REV;

    if( !g_ops.diagnosticMode                               /* We're not currently diagnosing something */
    &&  (   g_state.motorSteps < 0                          /* We're lost */
        ||  g_state.motorSteps > FIST_HEIGHT_MAX_STEP       /* We're lost */
        )
    )   return &ERR_MOT_POS_FIX_LOST;                       /* Tell them we.re lost */


    return nullptr;
}

Error* motorSetPositionAsZero() {
    m_motor.setCurrentPositionInSteps(0);
    return motorGetPosition();
}

Error ERR_MOT_SPEED_TOO_HIGH("motor target speed is too high", WARNING);
Error ERR_MOT_SPEED_TOO_LOW("motor target speed is too low", WARNING);
Error* motorSetSpeed(uint32_t stepsPerSec) {

    Error* err = nullptr;

    if( stepsPerSec > MOT_STEPS_PER_SEC_HIGH            /* We've been instructed poorly */
    ) { 
        stepsPerSec = MOT_STEPS_PER_SEC_HIGH;           // We know better
        err = &ERR_MOT_SPEED_TOO_HIGH;                  // We level a warning
    }

    else 
    if( stepsPerSec < 1                                 /* We've been instructed poorly */
    ) { 
        stepsPerSec = MOT_STEPS_PER_SEC_LOW;            // We know better
        err = &ERR_MOT_SPEED_TOO_LOW;                   // We level a warning
    }

    m_motor.setSpeedInStepsPerSecond(stepsPerSec);
    m_motor.setAccelerationInStepsPerSecondPerSecond(MOT_STEPS_PER_SEC_ACCEL);
    m_motor.setDecelerationInStepsPerSecondPerSecond(MOT_STEPS_PER_SEC_ACCEL);

    return err;
}

// Error ERR_MOT_TARGET_ZERO("motor target steps must not be zero");
Error* motorSetCourse(int32_t steps) {  
    
    // if( steps == 0                                      /* We can't step to that */
    // )   return &ERR_MOT_TARGET_ZERO;                    // We must protest

    m_motor.setTargetPositionRelativeInSteps(steps); 

    return nullptr;
}

bool motorTargetReached() { 
    return (m_motor.getDistanceToTargetSigned() == 0); 
}

void setupMotor() {
    m_motor.connectToPins(doutMotStep.pin, doutMotDir.pin);
    m_motor.startAsService(MOT_SERVICE_CORE);
    motorOff();
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
    doutMotor.checkPin();
}

void setupIO() {

    setupInterrupts();

    setupDigitalOutputs();
    
    setupMotor();
    
    checkStateIOPins();
}

