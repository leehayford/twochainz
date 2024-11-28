#include "x_machine.h"

uint32_t m_ui32MotorSpeed = MOT_STEPS_PER_SEC_HIGH;
int32_t m_i32MotorTargetSteps = 0;

void moveFistDown() {

    if(motorGetPosition() <= 0) {
        // WE'RE LOST...  GO SLOW... 
        m_ui32MotorSpeed = MOT_STEPS_PER_SEC_LOW;
        m_i32MotorTargetSteps = MOT_STEPS_PER_SEC_LOW * -1;
    }
    else {
        // FULL RIP
        m_ui32MotorSpeed = MOT_STEPS_PER_SEC_HIGH;
        m_i32MotorTargetSteps = motorGetPosition() * -1;
    }

    if(!g_state.motorOn || motorTargetReached()) {
        motorOn();
        motorSetSpeed(m_ui32MotorSpeed);
        motorMoveRelativeSteps(m_i32MotorTargetSteps);
    }
}

void moveFistUp() {
    
    if(motorGetPosition() > FIST_HEIGHT_MAX_STEP) {
        // WE'RE LOST...  GO SLOW... 
        m_ui32MotorSpeed = MOT_STEPS_PER_SEC_LOW;
        m_i32MotorTargetSteps = MOT_STEPS_PER_SEC_LOW;
    }
    else {
        // FULL RIP
        m_ui32MotorSpeed = MOT_STEPS_PER_SEC_HIGH;
        m_i32MotorTargetSteps = (g_config.height / FIST_INCH_PER_REV) * MOT_STEP_PER_REV;;
    }

    if(!g_state.motorOn || motorTargetReached()) {
        motorOn();
        motorSetSpeed(m_ui32MotorSpeed);
        motorMoveRelativeSteps(m_i32MotorTargetSteps);
    }

}

/* ALARMS *********************************************************************************************/

void statusUpdate(const char* status_msg) {
    g_state.setStatus(status_msg);
    setMQTTPubFlag(PUB_STATE);
}

bool checkIsBrakeEngaged() {
    return (
        g_state.breakOn // solenoid valve opened 
        &&
        !g_state.pressure // pressure has been released 
    );
}

bool checkIsHome() {
    return ( 
        g_state.anvilLimit // hammer is on the anvil
        && 
        g_state.fistLimit // fist is on the hammer
    );
}


bool checkIsRunning() {
    return ( 
        ( 
            ( g_config.cycles > 0 ) // we were told to swing the hammer 
            && 
            ( g_state.cyclesCompleted < g_config.cycles ) // we haven't finished swinging the hammer
        ) 
        ||
        !checkIsHome() // for some other reason, we are not safely at home
    );
}


#define N_OP_FLAGS 10
typedef enum {
    OP_ESTOP = 0,
    OP_AWAIT_DOOR_CLOSE,
    OP_AWAIT_CONFIG,

    OP_FIND_HAMMER,
    OP_FIND_ANVIL,
    OP_GO_HOME,

    OP_BRAKE_ENABLE,
    OP_BRAKE_DISABLE,
    OP_RAISE_HAMMER,
    OP_DROP_HAMMER
} eOpFlagMap_t;
uint32_t m_aui32OpFlags[N_OP_FLAGS];

bool checkEStop() {
    
    if(m_aui32OpFlags[OP_ESTOP] && !g_state.eStop) {
        /* TODO: CHECK IF SAFE TO CLEAR */
        m_aui32OpFlags[OP_ESTOP] = 0;
    }

    if(g_state.eStop) { 
        brakeOn(); 
        motorOff();
        magnetOff(); 

        if(m_aui32OpFlags[OP_ESTOP] == 0){
            statusUpdate(STATUS_ESTOP);
            m_aui32OpFlags[OP_ESTOP] = 1; 
            m_aui32OpFlags[OP_GO_HOME] = 1;
        }
    }
    
    return m_aui32OpFlags[OP_ESTOP]; 
}

bool checkDoor() {
    
    if(g_state.doorOpen) {
        brakeOn(); 
        motorOff();
        if(m_aui32OpFlags[OP_AWAIT_DOOR_CLOSE] == 0) {
            statusUpdate(STATUS_DOOR_OPEN);
            m_aui32OpFlags[OP_AWAIT_DOOR_CLOSE] = 1;
            m_aui32OpFlags[OP_GO_HOME] = 1;
        }

    } else if(m_aui32OpFlags[OP_AWAIT_DOOR_CLOSE]) {
        /* TODO: CHECK IF SAFE TO CLEAR */
        m_aui32OpFlags[OP_AWAIT_DOOR_CLOSE] = 0;
    }

    return m_aui32OpFlags[OP_AWAIT_DOOR_CLOSE];
}

bool checkFindHammer() { 

    if(m_aui32OpFlags[OP_FIND_HAMMER]) {

        if(g_state.fistLimit) {
            motorOff();
            magnetOn();
            brakeOff();
            m_aui32OpFlags[OP_FIND_HAMMER] = 0;
        }

        else moveFistDown();

    }
    
    return m_aui32OpFlags[OP_FIND_HAMMER];
}

bool checkFindAnvil() {

    if(m_aui32OpFlags[OP_FIND_ANVIL]) {

        if(g_state.anvilLimit) {
            magnetOn();
            brakeOff();
            m_aui32OpFlags[OP_FIND_ANVIL] = 0;
        } 

        else moveFistDown();

    }
    
    return m_aui32OpFlags[OP_FIND_ANVIL];
}

bool checkGoHome() {

    if(m_aui32OpFlags[OP_GO_HOME]) {

        if(!g_state.fistLimit) {
            m_aui32OpFlags[OP_FIND_HAMMER] = 1;
            statusUpdate(STATUS_SEEK_HAMMER);
        }

        else if(!g_state.anvilLimit) {
            m_aui32OpFlags[OP_FIND_ANVIL] = 1;
            statusUpdate(STATUS_SEEK_ANVIL);
        }
        
        else { // WE'RE HOME; CLEAR THE FLAG
            motorOff();
            motorSetPositionAsZero();
            m_aui32OpFlags[OP_GO_HOME] = 0;
        }

    } 

    return m_aui32OpFlags[OP_GO_HOME];
}

bool checkConfiguration() {

    if(m_aui32OpFlags[OP_AWAIT_CONFIG] && g_config.run) {
        /* TODO: CHECK IF SAFE TO CLEAR */
        m_aui32OpFlags[OP_AWAIT_CONFIG] = 0;
    }

    if(!g_config.run) {
        statusUpdate(STATUS_AWAIT_CONFIG);
        m_aui32OpFlags[OP_AWAIT_CONFIG] = 1;
    } 

    return m_aui32OpFlags[OP_AWAIT_CONFIG];
}

bool checkRaiseHammer() { /* TODO */

    return m_aui32OpFlags[OP_RAISE_HAMMER];
}

bool checkDropHammer() { /* TODO */

    return m_aui32OpFlags[OP_DROP_HAMMER];
}


int32_t m_i32LastAlarmCheck = 0; 
int32_t m_i32AlarmCheckPeriod_mSec = 5000; 
void checkIOAlarms() {

    // If we haven't sent state updated in a while, do it 
    if(g_ui32InterruptFlag == 0 && m_i32LastAlarmCheck + m_i32AlarmCheckPeriod_mSec < millis()) {
        checkAllLimits();
        setMQTTPubFlag(PUB_STATE);
        g_ui32InterruptFlag = 1;
    }

    if (g_ui32InterruptFlag > 0) { 
        m_i32LastAlarmCheck = millis();
        g_ui32InterruptFlag = 0;
        
        if(checkEStop()) // SKIP EVERYTHING ELSE UNTIL SYSTEM IS ENABLED  
            return; 

        // SYSTEM IS ENABLED; CONTINUE
        
        if(checkDoor()) // SKIP EVERYTHING ELSE UNTIL DOOR IS CLOSED 
            return; 

        // DOOR IS CLOSED; CONTINUE

        if(checkFindHammer()) // SKIP EVERYTHING ELSE UNTIL HAMMER IS FOUND 
            return;

        // HAMMER IS FOUND; CONTINUE

        if(checkFindAnvil()) // SKIP EVERYTHING ELSE UNTIL ANVIL IS FOUND
            return;

        // ANVIL IS FOUND; CONTINUE

        if(checkGoHome()) // SKIP EVERYTHING ELSE UNTIL MACHINE IS IN HOME POSITION
            return;

        // MACHINE IS IN HOME POSITION; CONTINUE

        if(checkConfiguration()) // SKIP EVERYTHING ELSE UNTIL CONFIGURED TO RUN 
            return;

        // CONFIGURED TO RUN; CONTINUE 

        if(checkRaiseHammer()) // SKIP EVERYTHING ELSE UNTIL HAMMER IS RAISED 
            return;

        // HAMMER IS RAISED; CONTINUE

        if(checkDropHammer()) // SKIP EVERYTHING ELSE UNTIL HAMMER IS DROPPED
            return;

        // HAMMER IS DROPPED; CONTINUE
        
    }
}