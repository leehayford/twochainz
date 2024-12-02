#include "x_io.h"
 
/* INTERRUPTS *****************************************************************************************/

ITRPin itrPins[N_ITR_PINS] = {

    {   {PIN_ITR_ESTOP, &g_state.eStop, ITR_PIN_ACTIVE_LOW},  
        [](){   debounce(CHK_ESTOP);    }
    }, 
    
    {   {PIN_ITR_DOOR, &g_state.doorOpen, ITR_PIN_ACTIVE_HIGH},  
        [](){   debounce(CHK_DOOR);     }
    }, 
    
    {   {PIN_ITR_FIST, &g_state.fistLimit, ITR_PIN_ACTIVE_LOW},  
        [](){   debounce(CHK_FIST);     }
    }, 
    
    {   {PIN_ITR_ANVIL, &g_state.anvilLimit, ITR_PIN_ACTIVE_LOW},  
        [](){   debounce(CHK_ANVIL);    }
    }, 
    
    {   {PIN_ITR_HOME, &g_state.homeLimit, ITR_PIN_ACTIVE_LOW},  
        [](){   debounce(CHK_HOME);     }
    }, 
    
    {   {PIN_ITR_TOP, &g_state.topLimit, ITR_PIN_ACTIVE_LOW},  
        [](){   debounce(CHK_TOP);      }
    }, 

    {   {PIN_ITR_PRESSURE, &g_state.pressure, ITR_PIN_ACTIVE_LOW},  
        [](){   debounce(CHK_PRESSURE); }
    }
};

 void IRAM_ATTR debounce(eItrCheckMap_t eChk) { 

    if( itrPins[eChk].obj.checkTime == 0      /* This a new interrupt event */
    ) {                                                 
        itrPins[eChk].obj.checkTime = millis() 
        + itrPins[eChk].obj.debouncePeriod;   // Schedule the debounce
    }
}   

uint32_t m_ui32NowMillis;
void IRAM_ATTR isrDebounceTimerFunc() {

    m_ui32NowMillis = millis();                     // Get the time right now; once

    for(ITRPin &itr : itrPins                         /* Loop through the interrupt pins */
    ) {                       
        itr.obj.itrCheck(m_ui32NowMillis);          // Check 'em
    }
}

bool g_ui32InterruptFlag = false;
hw_timer_t *m_phwTimerDebounce = NULL;
void setupInterrupts() {

    g_ui32InterruptFlag = false;                    // Initialize global interrupt flag

    for(ITRPin &itr : itrPins) {                      // Initialize interrupt pins

        pinMode(itr.obj.pin, INPUT_PULLUP);         // This pin is an input with internal pull-up

        attachInterrupt(
            digitalPinToInterrupt(itr.obj.pin),     // This pin is an interrupt
            itr.func,                               // This is the interrupt service routine
            CHANGE                                  // Call it on any change in state
        );
    }

    m_phwTimerDebounce = timerBegin(
        ITR_DEBOUNCE_TIMER, 
        ITR_DEBOUNCE_PRESCALE, 
        ITR_DEBOUNCE_COUNT_UP
    );
    
    timerAttachInterrupt(
        m_phwTimerDebounce, 
        isrDebounceTimerFunc, 
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

DOUTPin outPins[N_DOUT_PINS] = {

    {   PIN_OUT_BRAKE, &g_state.breakOn, DOUT_PIN_ACTIVE_LOW
    },

    {   PIN_OUT_MAGNET, &g_state.magnetOn, DOUT_PIN_ACTIVE_HIGH
    },

    {   PIN_OUT_MOT_EN, &g_state.motorOn, DOUT_PIN_ACTIVE_LOW
    },

    {   PIN_OUT_MOT_STEP,
    },

    {   PIN_OUT_MOT_DIR,
    }
};

void setupDigitalOutputs() {
    
    for(DOUTPin &out : outPins                      /* Initialize digital output pins */
    ) {                      
        pinMode(out.pin, OUTPUT);                   // This pin is an output 
    }
}

/* RELAY CONTROL *** END ***************************************************************************/



/* MOTOR CONTROL ***********************************************************************************/

ESP_FlexyStepper m_motor;

void motorSetPositionAsZero() {
    m_motor.setCurrentPositionInSteps(0);
}

bool motorTargetReached() { 
    return (m_motor.getDistanceToTargetSigned() == 0); 
}

void motorGetPosition() {

    g_state.motorSteps = m_motor.getCurrentPositionInSteps();

    g_state.currentHeight = 
        ((float)g_state.motorSteps / MOT_STEP_PER_REV)  // Convert steps to revolutions
        * MOT_INCH_PER_REV;                            // Convert revolutions to inches
}

void motorSetSpeed(int stepsPerSec) {
    if( stepsPerSec > MOT_STEPS_PER_SEC_HIGH            /* We've been instructed poorly */
    ) { 
        stepsPerSec = MOT_STEPS_PER_SEC_HIGH;           // We know better
    }
    m_motor.setSpeedInStepsPerSecond(stepsPerSec);
    m_motor.setAccelerationInStepsPerSecondPerSecond(MOT_STEPS_PER_SEC_ACCEL);
    m_motor.setDecelerationInStepsPerSecondPerSecond(MOT_STEPS_PER_SEC_ACCEL);
}

void motorMoveRelativeSteps(int32_t steps, uint32_t stepsPerSec) { 
    outPins[DOUT_MOTOR].enable();                                       
    motorSetSpeed(stepsPerSec);                      
    m_motor.setTargetPositionRelativeInSteps(steps); 
}

void setupMotor() {
    m_motor.connectToPins(outPins[DOUT_MOT_STEP].pin, outPins[DOUT_MOT_DIR].pin);
    m_motor.startAsService(MOT_SERVICE_CORE);
    outPins[DOUT_MOTOR].disable();
}

/* MOTOR CONTROL *** END *************************************************************************/



void checkStateIOPins() {

    for(DOUTPin &out : outPins) {
        out.checkPin();
    }

    for(ITRPin &itr : itrPins) {
        itr.obj.checkPin();
    }
}

void setupIO() {

    setupInterrupts();

    setupDigitalOutputs();

    setupMotor();
}



/* TODO: UNDEFINE TEST_STEP_DRIVER FOR PRODUCTION */ 
#ifdef TEST_STEP_DRIVER

int32_t m_i32LastPosUpdate = 0;
void testMotorGetPosition() {

    g_state.currentHeight = (
        (float)m_motor.getCurrentPositionInSteps() 
        / MOT_STEP_PER_REV
        ) 
        * MOT_INCH_PER_REV;

    m_i32LastPosUpdate = millis();
    // setMQTTPubFlag(PUB_OPS_POS);
}

int32_t m_i32PosUpdatePeriod_mSec = 500;
int32_t m_i32StepsTarget = 0;
void motorBackNForth() {
    if(!g_config.run) 
        return;

    else if(g_ops.cyclesCompleted <= g_config.cycles) {

        if (m_motor.getDistanceToTargetSigned() == 0) {

            testMotorGetPosition();
            
            if(m_i32StepsTarget == 0) { // We are starting the fist cycle
                m_i32StepsTarget = ( g_config.height / MOT_INCH_PER_REV ) * MOT_STEP_PER_REV;
                motorSetSpeed(MOT_STEPS_PER_SEC_HIGH);
                // motorOn();
                outPins[DOUT_MOTOR].enable();
                g_ops.setStatus((char*)"BEGIN...");
                // setMQTTPubFlag(PUB_STATE);
            } 
            else if(m_i32StepsTarget < 0) { // We just completed a cycle (down-stroke)
                g_ops.cyclesCompleted++;

                if(g_ops.cyclesCompleted == g_config.cycles) { // That was the last cycle
                    // motorOff();
                    outPins[DOUT_MOTOR].disable();
                    g_ops.setStatus((char*)"DONE");
                    // setMQTTPubFlag(PUB_STATE);
                    g_config.run = false;
                    m_i32StepsTarget = 0;
                    return; 
                }

                // Go again (up-stroke)
                g_ops.setStatus((char*)"GOING UP");
                // magnetOn();
                // brakeOff();
                outPins[DOUT_MAGNET].enable();
                outPins[DOUT_BRAKE].disable();

            } else { 
                // We're at the top; DROP THE HAMMER!... and then go get it.
                g_ops.setStatus((char*)"GOING DOWN");
                // magnetOff();
                // brakeOn();
                outPins[DOUT_MAGNET].disable();
                outPins[DOUT_BRAKE].enable(); 
            }

            // setMQTTPubFlag(PUB_STATE);

            delay(100); // Serial.printf("\nm_steps: %d", m_i32StepsTarget);
            m_i32StepsTarget *= -1;
            m_motor.setTargetPositionRelativeInSteps(m_i32StepsTarget);

        } 

        // Check if we should send a motor position update
        if (m_i32LastPosUpdate <= (millis() + m_i32PosUpdatePeriod_mSec)) 
            testMotorGetPosition();

        
    }
    
}
#endif /* TEST_STEP_DRIVER */
