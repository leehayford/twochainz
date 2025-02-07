#include "x_io.h"


/* ADMIN SETTINGS *************************************************************************************/

void writeAdminSettingsToFile() {
    writeToFile(ADMIN_DEFAULT_FILE, g_admin.serializeToJSON());
}

void validateAdminSettings(char* data) {
    // Serial.printf("\nvalidateAdminSettings ...\n");

    try {

    g_admin.parseFromJSON(data);
    } catch (...) { 
        Serial.printf("\nvalidateAdminSettings : FAILED\n");
        throw -1;
    }

    changeHammerTimeoutPeriod();
    changeITRDebounceTimerPeriod();
    changeBrakeTimeoutPeriod();

    Serial.printf("\nvalidateAdminSettings : OK\n");
}

void validateAdminSettingsFile() {
    // Serial.printf("\nvalidateAdminSettingsFile ...\n");

    if( !fileExists(ADMIN_DEFAULT_FILE)         /* This is the first time this ESP has been run */
    ) {  
        Serial.printf("\nvalidateAdminSettingsFile -> file not found; creating...\n");
        writeAdminSettingsToFile();             // Create the default admin settings file 
    }
    else
        Serial.printf("\nvalidateAdminSettingsFile -> check existis : OK...\n");

    char data[MQTT_PUB_BUFFER_SIZE] = "";
    readFromFile(data, ADMIN_DEFAULT_FILE);

    Serial.printf("\nvalidateAdminSettingsFile -> read from file : OK\n");

    if( strlen(data)                        /* The string length of the file */
    !=  strlen(g_admin.serializeToJSON())   /* The string length of model_admin (as defined currently)*/
    ) {   
        Serial.printf("\nvalidateAdminSettingsFile -> file length : %d : INVALID\n", strlen(data));
        throw -1;                           // We refuse to load the file
    }
    Serial.printf("\nvalidateAdminSettingsFile -> file length : %d : OK\n", strlen(data));

    try {
    validateAdminSettings(data);
    } catch (...) { 
        Serial.printf("\nvalidateAdminSettingsFile : FAILED\n");  
        throw -1;
    }
    Serial.printf("\nvalidateAdminSettingsFile : OK\n\n");
}

void setupAdminSettingsFile() {
    
    try {                                   /* Try loading the default admin settings */
        validateAdminSettingsFile();        
    } 
    catch (...) {                           /* The admin file in memory is invalid */
        Serial.printf("\n\n******* DELETING INVALID ADMIN FILE *******\n\n");
        deleteFile(ADMIN_DEFAULT_FILE);     // Destrot it, lest we look like fools!
        
        validateAdminSettingsFile();        // Save and load the model_admin (as defined currently) 
    }
}

/* ADMIN SETTINGS *** END *****************************************************************************/



/* INTERRUPTS *****************************************************************************************/

ITRPin itrpEStop(PIN_ITR_ESTOP, &g_state.eStop, ITR_PIN_ACTIVE_LOW);
ITRPin itrpDoor(PIN_ITR_DOOR, &g_state.doorOpen, ITR_PIN_ACTIVE_HIGH);
ITRPin itrpFist(PIN_ITR_FIST, &g_state.fistLimit, ITR_PIN_ACTIVE_LOW);
ITRPin itrpAnvil(PIN_ITR_ANVIL, &g_state.anvilLimit, ITR_PIN_ACTIVE_LOW);
ITRPin itrpHome(PIN_ITR_HOME, &g_state.homeLimit, ITR_PIN_ACTIVE_LOW);
ITRPin itrpTop(PIN_ITR_TOP, &g_state.topLimit, ITR_PIN_ACTIVE_LOW);
ITRPin itrpPressure(PIN_ITR_PRESSURE, &g_state.pressure, ITR_PIN_ACTIVE_LOW);

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



/* TIMERS ********************************************************************************************/

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

/* HAMMER-TIME TIMER */
hw_timer_t *tmrHammerStrike = NULL;
void setupHammerTimer() {
    tmrHammerStrike = timerBegin(
        OPS_HAMMER_TIMER,
        OPS_HAMMER_TIMER_PRESCALE,
        OPS_HAMMER_TIMER_COUNT_UP
    );

    timerAttachInterrupt(
        tmrHammerStrike,
        [](){ 
            g_state.hammerTimeout = true; 
            g_state.stateChangeFlag = true;
        },
        OPS_HAMMER_TIMER_EDGE
    );

    timerAlarmWrite(
        tmrHammerStrike,
        OPS_HAMMER_TIMER_STRIKE_PERIOD_uSEC,
        OPS_HAMMER_TIMER_RUN_ONCE
    );
    
}

void startHammerTimer() {
    g_state.hammerTimeout = false;              // Clear the flag
    timerRestart(tmrHammerStrike);              // Necessary for calls subsequent to the first call
    timerAlarmEnable(tmrHammerStrike);          // Hammertime is upon us
}

void changeHammerTimeoutPeriod() {
    timerAlarmWrite(
        tmrHammerStrike,
        g_admin.opsTmrHammer_uSec,
        OPS_HAMMER_TIMER_RUN_ONCE
    );
}

/* BRAKE PRESSURE TIMER */
hw_timer_t *tmrBrakePressure = NULL;
void setupBrakeTimer() {
    tmrBrakePressure = timerBegin(
        OPS_BRAKE_TIMER,
        OPS_BRAKE_TIMER_PRESCALE,
        OPS_BRAKE_TIMER_COUNT_UP
    );

    timerAttachInterrupt(
        tmrBrakePressure,
        [](){ 
            g_state.brakeTimeout = true;  
            g_state.stateChangeFlag = true;
        },
        OPS_BRAKE_TIMER_EDGE
    );

    timerAlarmWrite(
        tmrBrakePressure,
        OPS_BRAKE_TIMER_PERIOD_uSEC,
        OPS_BRAKE_TIMER_RUN_ONCE
    );

}

void startBrakeTimer() {
    g_state.brakeTimeout = false;               // Clear the flag
    timerRestart(tmrBrakePressure);             // Necessary for calls subsequent to the first call
    timerAlarmEnable(tmrBrakePressure);         // Brake pressure should now be as expected
}

void changeBrakeTimeoutPeriod() {
    timerAlarmWrite(
        tmrHammerStrike,
        g_admin.opsTmrHammer_uSec,
        OPS_HAMMER_TIMER_RUN_ONCE
    );
}

void setupTimers() {
    setupITRDebounceTimer();
    setupHammerTimer();
    setupBrakeTimer();
}

/* TIMERS *** END ***********************************************************************************/



/* DIGITAL OUT **************************************************************************************/
DOUTPin doutBrake(PIN_OUT_BRAKE, &g_state.brakeOn, DOUT_PIN_ACTIVE_LOW);
void brakeOn() { 
    doutBrake.enable(); 
    startBrakeTimer();
}      
void brakeOff() { 
    doutBrake.disable(); 
    startBrakeTimer();
}    

DOUTPin doutMagnet(PIN_OUT_MAGNET, &g_state.magnetOn, DOUT_PIN_ACTIVE_HIGH);
void magnetOn() { doutMagnet.enable(); }    
void magnetOff() { doutMagnet.disable(); }  

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

void motorGetPosition() {

    g_state.motorSteps = m_motor.getCurrentPositionInSteps();

    g_state.currentHeight = 
        ((float)g_state.motorSteps / g_admin.motStepsRev) 
        * g_admin.motInchRev;
}

void motorSetPositionAsZero() {
    // Serial.printf("motorSetPositionAsZero() dist 2 target -> BEFORE: %d ", m_motor.getDistanceToTargetSigned());
    m_motor.setCurrentPositionInSteps(0);
    m_motor.setTargetPositionRelativeInSteps(0);
    // Serial.printf("motorSetPositionAsZero() dist 2 target -> AFTER: %d", m_motor.getDistanceToTargetSigned());
    motorGetPosition();
}

void motorSetSpeed(uint32_t stepsPerSec) {

    if( stepsPerSec > g_admin.motHzHigh                 /* We've been instructed poorly */
    )   stepsPerSec = g_admin.motHzHigh;                // We know better
    else 
    if( stepsPerSec < 1                                 /* We've been instructed poorly */
    )   stepsPerSec = g_admin.motHzLow;                 // We know better

    m_motor.setSpeedInStepsPerSecond(stepsPerSec);
    m_motor.setAccelerationInStepsPerSecondPerSecond(g_admin.motAccel);
    m_motor.setDecelerationInStepsPerSecondPerSecond(g_admin.motDecel);
}

void motorSetCourse(int32_t steps) { 
    m_motor.setTargetPositionRelativeInSteps(steps); 
}

void motorStop() { m_motor.emergencyStop(); }

bool motorTargetReached() { return (m_motor.getDistanceToTargetSigned() == 0); }

void setupMotor() {
    m_motor.connectToPins(doutMotStep.pin, doutMotDir.pin);
    m_motor.startAsService(MOT_SERVICE_CORE);
    motorStop();
}

/* MOTOR CONTROL *** END *************************************************************************/



void setupIO() {

    setupTimers();

    setupInterrupts();

    setupDigitalOutputs();
    
    setupMotor();
    
    // Check initial pin states
    itrpEStop.checkPin();
    itrpDoor.checkPin();
    itrpFist.checkPin();
    itrpAnvil.checkPin();
    itrpHome.checkPin();
    itrpTop.checkPin();
    itrpPressure.checkPin();
    doutBrake.checkPin();
    doutMagnet.checkPin();

    setupAdminSettingsFile();
    
}

