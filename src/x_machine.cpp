#include "x_machine.h"

/* TDOD: SETUP TIMER */
// int32_t m_i32LastPosUpdate = 0;
// int32_t m_i32PosUpdatePeriod_mSec = 500; 

void statusUpdate(const char* status_msg) {
    g_ops.setStatus(status_msg);
    setMQTTPubFlag(PUB_STATE);
}


int32_t m_i32MotorTargetSteps = 0;
uint32_t m_ui32MotorSpeed = MOT_STEPS_PER_SEC_HIGH;
void motorSetCoursAndSpeed() {
   
    motorGetPosition();                                 // Find out where we think we are

    if( g_state.motorSteps < 0                          /* We're lost */
    ||  g_state.motorSteps > MOT_HEIGHT_MAX_STEP        /* We're lost */
    ||  g_ops.recovery                                  /* We're lost and reassissing */
    ) {  
        m_ui32MotorSpeed = MOT_STEPS_PER_SEC_LOW;       // Set speed to *** SLOW ***
        m_i32MotorTargetSteps = MOT_RECOVERY_STEPS;     // Do 1/4 revolution (DOWN) and reassess

        if( !g_ops.recovery                             /* We didn't know we were in recovery */
        ) {   
            g_ops.awaitHelp = true;                     // We need an operator to allow to continue 
            g_ops.seekHome = true;                      // When they do, go home 1/4 revolution at a time         
            g_ops.recovery = true;                      // Come back and reassess each time until get there 
            statusUpdate(STATUS_RECOVERY);              // Sing it
        }

        if( !g_ops.seekHome                             /* We've been home */
        &&  g_state.motorSteps == 0                     /* We're still there */
        ) {
            g_ops.recovery = false;                     // Clear the recovery flag
        }
    }

    else {                                              // We assume we're on track 
        m_ui32MotorSpeed = MOT_STEPS_PER_SEC_HIGH;      // Set speed to *** FULL RIP ***
        
        if( g_ops.seekHammer                            /* We need to go DOWN */
        ||  g_ops.seekAnvil                             /* We need to go DOWN */
        ||  g_ops.seekHome                              /* We need to go DOWN */
        ) {  
            // Go DOWN to zero height
            m_i32MotorTargetSteps = g_state.motorSteps * -1;
        }

        else                                            /* We need to go UP */
        if( g_ops.raiseHammer
        ) {  
            // Go UP to configured height
            m_i32MotorTargetSteps = (g_config.height / MOT_INCH_PER_REV) * MOT_STEP_PER_REV;          
        }                    
    }

    if( !g_state.motorOn                                /* This is the first time through */ 
    ||  g_ops.recovery                                  /* We are lost and seeking home */   
    ) {                                                     
        // Get to steppin'  
        motorMoveRelativeSteps(m_i32MotorTargetSteps, m_ui32MotorSpeed);
    }
    

}

/* OPERATIONS *****************************************************************************************/

bool awaitEStopClear() {
    
    if( g_state.eStop                           /* The emergency stop button is pressed */
    ) {                                     
        outPins[DOUT_MOTOR].disable();          // Stop moving
        outPins[DOUT_BRAKE].enable();           // Apply the brake
        outPins[DOUT_MAGNET].disable();         // Turn off the magnet

        if( !g_ops.awaitEStop                   /* We did't know the button was pressed */
        ) { 
            g_ops.awaitEStop = true;            // Wait for the button to be reset
            g_ops.seekHome = true;              // Go home after that
            statusUpdate(STATUS_ESTOP);         // Send status to the world
        }
    }
        
    else                                        /* The system has been enabled */
    if( g_ops.awaitEStop                        /* We have yet to clear the flag */
    &&  !g_ops.seekHome                         /* We've already returned home */
    ) {
        g_ops.awaitEStop = false;               // Stop waiting for the system to be enabled
    }

    return g_ops.awaitEStop; 
}

bool awaitDoorClose() {
    
    if( g_state.doorOpen                        /* The door is open */
    ) {  
        outPins[DOUT_MOTOR].disable();          // Stop moving
        outPins[DOUT_BRAKE].enable();           // Apply the brake
        outPins[DOUT_MAGNET].disable();         // Turn off the magnet

        if( !g_ops.awaitDoor                    /* We did't know the door was open */
        ) { 
            g_ops.awaitDoor = true;             // Wait for the door to close
            g_ops.seekHome = true;              // Go home after that
            statusUpdate(STATUS_DOOR_OPEN);     // Send status to the world
        }
    } 
    
    else                                        /* The door is closed */
    if( g_ops.awaitDoor                         /* We have yet to clear the flag */
    &&  !g_ops.seekHome                         /* We've already returned home */
    ) {
        g_ops.awaitDoor = false;                // Stop waiting for the door to close
    }

    return g_ops.awaitDoor;
}

bool awaitHelp() {

    if( g_ops.awaitHelp                         /* Something bad happened and we need help */
    ) {

        if( !g_ops.reqHelp                      /* We have yet to ask for help */
        ) {
            g_ops.reqHelp = true;               // Ask for help
            statusUpdate(STATUS_REQUEST_HELP);  // Send status to the world
        }
    }

    return g_ops.awaitHelp;
}

/* Call this funtion to seek, or ensure we have, the hammer */
bool seekHammer() {                             

    if( g_state.fistLimit                   /* We found the hammer */
    ) {
        if( g_ops.seekHammer                /* We were seeking the hammer */
        ) {
            // motorOff();                     // Stop moving before we secure the hammer 
            outPins[DOUT_MOTOR].disable();  // Stop moving before we secure the hammer 
            g_ops.seekHammer = false;       // Stop seeking the hammer
        }
        outPins[DOUT_MAGNET].enable();      // Secure the hammer
    }

    else                                    /* We need the hammer */
    if( !g_ops.seekHammer                   /* We have yet to seek the hammer */
    ){
        g_ops.seekHammer = true;            // Seek the hammer
        statusUpdate(STATUS_SEEK_HAMMER);   // Sing it
    }

    return g_ops.seekHammer;
}

/* Call this funtion to seek, or ensure we have, the anvil */
bool seekAnvil() {

    if( g_state.anvilLimit                      /* We found the anvil */
    ) {
        if( g_ops.seekAnvil                     /* We were seeking the anvil */
        ) {
            g_ops.seekAnvil = false;            // Stop seeking the anvil
        }
        outPins[DOUT_MAGNET].enable();          // Make sure we're still holding the hammer
        outPins[DOUT_BRAKE].disable();          // Make sure the brake is still released
    }

    else                                        /* We need the anvil */
    if( !g_ops.seekAnvil                        /* We have yet to start looking for the anvil */
    ) {
        g_ops.seekAnvil = true;                 // Start looking for the anvil
        statusUpdate(STATUS_SEEK_ANVIL);        // Send status to the world
    } 

    return g_ops.seekAnvil;
}

bool checkGoHome() {

    if( g_ops.seekHome                          /* We are trying to go home */
    ) {

        if( seekHammer()                        /* We are seeking the hammer */
        ||  seekAnvil()                         /* We are seeking the anvil */
        ) {
            motorSetCoursAndSpeed();           // Get to steppin'
        }
        
        else                                    /* We're home */ 
        {
            // motorOff();                         // Stop moving
            outPins[DOUT_MOTOR].disable();      // Stop moving
            motorSetPositionAsZero();           // Reset our home position
            g_ops.seekHome = false;             // Stop trying to go home
        }

    } 

    return g_ops.seekHome;
}

bool checkConfiguration() {

    if( g_config.run                            /* We have been told to run */
    &&  g_config.cycles > 0                     /* We have a valid cycle setting */
    &&  g_config.height > 0
    &&  g_config.height < MOT_HEIGHT_MAX_INCH  /* We have a valid height setting */
    ) {
        if( g_ops.awaitConfig                   /* We were waiting for a valid configuration */
        ) {
            g_ops.awaitConfig = false;          // Stop waiting for a configuration
        }   
    }

    else                                        /* We need to wait for a valid configuration */
    if( !g_ops.awaitConfig                      /* We didn't know that until now */
    ){
        g_ops.awaitConfig = true;               // Start waiting
        statusUpdate(STATUS_AWAIT_CONFIG);      // Send status to the world
    } 

    return g_ops.awaitConfig;
}

bool raiseHammer() { /* TODO */

    g_ops.raiseHammer = true;
    motorSetCoursAndSpeed();                   // Get to steppin'

    return g_ops.raiseHammer;
}

bool dropHammer() { /* TODO */

    g_ops.dropHammer = true;
    return g_ops.dropHammer;
}

void runConfiguration() { /* TODO */

    if( g_ops.dropHammer                        /* We dropped the hammer */
    ) {
        // Wait for it to strike

        // After strike: 
        // - go home
        
        // - check cyclesCompleted
        //      Raise the hammer and strike again
        //      Or 
        //      Shut it down and wait for a new congig
    }


    if( g_ops.raiseHammer                        /* We dropped the hammer */
    ) {
        // Wait until we're ready to drop

        // After ready, drop
    }
}

hw_timer_t *m_phwTimerOpsInterrupt = NULL;
void setupOps() {

    m_phwTimerOpsInterrupt = timerBegin(
        1,
        80,
        true
    );

}

int32_t m_i32LastAlarmCheck = 0; 
int32_t m_i32AlarmCheckPeriod_mSec = 5000; 
void runOperations() {

    // If we haven't sent state updated in a while, do it 
    if(g_ui32InterruptFlag == 0 && m_i32LastAlarmCheck + m_i32AlarmCheckPeriod_mSec < millis()) {
        checkStateIOPins();
        setMQTTPubFlag(PUB_STATE);
        g_ui32InterruptFlag = 1;
    }

    if (g_ui32InterruptFlag > 0) { 
        m_i32LastAlarmCheck = millis();
        g_ui32InterruptFlag = 0;
        
        if(awaitEStopClear()) // SKIP EVERYTHING ELSE UNTIL SYSTEM IS ENABLED  
            return; 

        // SYSTEM IS ENABLED; CONTINUE
        
        if(awaitDoorClose()) // SKIP EVERYTHING ELSE UNTIL DOOR IS CLOSED 
            return; 

        // DOOR IS CLOSED; CONTINUE

        if(awaitHelp()) // SKIP EVERYTHING ELSE UNTIL WE GET HELP
            return;

        // WE HAVE BEEN HELPED; CONTINUE

        if(checkGoHome()) // SKIP EVERYTHING ELSE UNTIL MACHINE IS IN HOME POSITION
            return;

        // MACHINE IS IN HOME POSITION; CONTINUE

        if(checkConfiguration()) // SKIP EVERYTHING ELSE UNTIL CONFIGURED TO RUN 
            return;

        // CONFIGURED TO RUN; CONTINUE 

        runConfiguration(); // RUN CONFIGURATION 
        
    }
}