#include "x_machine.h"

/* FUNCTION PROTOTYPES : USED BY alarmCheckXXXX*/
void moveToHome();
void moveToHammer();
void moveToSwingHeight();
void emergencyStop();

/* ALARMS *********************************************************************************************/

bool m_bAlarmRaised = false;

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
        &&
        !g_state.motorOn // motor has been disabled
        &&
        checkIsBrakeEngaged() 
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

bool checkDoorAlarm() {
    return ( g_state.doorOpen && (
            checkIsRunning() // we are swinging the hammer
            || 
            !checkIsHome() // we haven't returned to home, or we're stuck in some dangerous position
        )
    );
}

void doEStop() {
    if ( !checkIsBrakeEngaged( ) ) 
        brakeOn(); 
}



int32_t m_i32LastAlarmCheck = 0; 
int32_t m_i32AlarmCheckPeriod_mSec = 5000; 
void checkIOAlarms() {
    if(m_i32LastAlarmCheck + m_i32AlarmCheckPeriod_mSec < millis() ) {
        checkAllLimits();
        g_ui32InterruptFlag = 1;
    }
    if (g_ui32InterruptFlag > 0) { // Serial.printf("\ng_ui32InterruptFlag: %d", g_ui32InterruptFlag);
        g_ui32InterruptFlag = 0;

        // if ( g_state.eStop ) {
        //     brakeOn();
        //     motorOff();
        //     magnetOff();
        //     statusUpdate(STATUS_ESTOP);
        //     return;
        // }

        // if ( checkDoorAlarm( ) ) {
        //     brakeOn();
        //     motorOff();
        //     if ( g_state.fistLimit )
        //         magnetOn();
        //     statusUpdate(STATUS_DOOR_OPEN);
        //     return;
        // }

        if(g_config.cycles == 0 || g_state.cyclesCompleted == g_config.cycles){ 
            statusUpdate(STATUS_READY);
            setMQTTPubFlag(PUB_CONFIG);
        }
        m_i32LastAlarmCheck = millis();
    }
}