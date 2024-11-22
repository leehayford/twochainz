#include "x_machine.h"

/* FUNCTION PROTOTYPES : USED BY alarmCheckXXXX*/
void moveToHome();
void moveToHammer();
void moveToSwingHeight();
void emergencyStop();

/* ALARMS *********************************************************************************************/
int checkAlarmsFlag = 0;
void statusUpdate(const char* status_msg) {
    sta.setStatus(status_msg);
    pubs[PUB_STATE].flag = 1;
}

void alarmCheckEStop() { /* TODO: */
    if( sta.emergencyStop ) { 
        statusUpdate(STATUS_ESTOP); 
    }
}
void alarmCheckDoor() { /* TODO: */
    if( sta.doorClosed ) { 
        statusUpdate(STATUS_DOOR_OPEN); 
    }
}
void alarmCheckFist() { /* TODO: */
    if( sta.fistContact ) { 
        statusUpdate(STATUS_FIST_OPEN); 
    }
}
void alarmCheckAnvil() { /* TODO: */
    if( sta.anvilContact ) { 
        statusUpdate(STATUS_ANVIL_LIMIT); 
    }
}
void alarmCheckTop() { /* TODO: */
    if( sta.topContact ) { 
        statusUpdate(STATUS_TOP_LIMIT); 
    }
}
void alarmCheckPressure() { /* TODO: */
    if( sta.pressureContact ) { 
        statusUpdate(STATUS_PRESSURE); 
    }
}

void checkIOAlarms() {
    if (checkAlarmsFlag > 0) {
        alarmCheckEStop();
        alarmCheckDoor();
        alarmCheckFist();
        alarmCheckAnvil();
        alarmCheckTop();
        alarmCheckPressure();
        checkAlarmsFlag = 0;
    } 
}