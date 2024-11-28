#include "x_machine.h"

uint32_t m_ui32MotorSpeed = MOT_STEPS_PER_SEC_HIGH;
int32_t m_i32MotorTargetSteps = 0;
int32_t m_i32MotorPosSteps = 0;
int32_t m_i32LastPosUpdate = 0;
int32_t m_i32PosUpdatePeriod_mSec = 500;

void motorSetSpeedAndCourse() {
   
    m_i32MotorPosSteps = motorGetPosition();            // Where do we think we are?

    if( m_i32MotorPosSteps < 0                          /* We're lost */
    ||  m_i32MotorPosSteps > FIST_HEIGHT_MAX_STEP       /* We're lost */
    ||  g_ops.recovery                                  /* We're lost and reassissing */
    ) {  
        m_ui32MotorSpeed = MOT_STEPS_PER_SEC_LOW;       // Go *** SLOW ***
        m_i32MotorTargetSteps = MOT_REVOVERY_STEPS * -1;// Do 1/4 revolution and reassess

        if( !g_ops.recovery                             /* Ensure we set flags only once */
        ) {   
            g_ops.awaitHelp = true;                     // We need an operator to tell us it's ok to continue            
            g_ops.recovery = true;                      // Come back and reassess until we've been home
            g_ops.seekHome = true;                      // GO HOME 
            statusUpdate(STATUS_RECOVERY);              // Send status to the world
        }

        if( g_ops.seekHome                              /* We've been home */
        &&  m_i32MotorPosSteps                          /* We're still there */
        ) {
            g_ops.recovery = false;                     // Clear the recovery flag
        }
    }

    else {                                              // We're probably on track 
        m_ui32MotorSpeed = MOT_STEPS_PER_SEC_HIGH;      // Go *** FULL RIP ***
        
        if( g_ops.seekHammer                            /* We need to go DOWN */
        ||  g_ops.seekAnvil                             /* We need to go DOWN */
        ||  g_ops.seekHome                              /* We need to go DOWN */
        ) {  
            // Go DOWN to zero height
            m_i32MotorTargetSteps = motorGetPosition() * -1;
        }

        else                                            /* We need to go UP */
        if( g_ops.raiseHammer
        ) {  
            // Go UP to configured height
            m_i32MotorTargetSteps = (g_config.height / FIST_INCH_PER_REV) * MOT_STEP_PER_REV;          
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

void statusUpdate(const char* status_msg) {
    g_state.setStatus(status_msg);
    setMQTTPubFlag(PUB_STATE);
}

bool checkEStop() {
    
    if( g_state.eStop                           /* The emergency stop button is pressed */
    ) {                                     
        motorOff();                             // Stop moving
        brakeOn();                              // Apply the brake
        magnetOff();                            // Turn off the magnet

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

bool checkDoor() {
    
    if( g_state.doorOpen                        /* The door is open */
    ) {  
        motorOff();                             // Stop moving
        brakeOn();                              // Apply the brake
        magnetOff();                            // Turn off the magnet

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

bool checkHelp() {

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

bool seekHammer() { 

    if( g_state.fistLimit                   /* We found the hammer */
    &&  g_ops.seekHammer                    /* We were looking for the hammer */
    ) {
        motorOff();                         // Stop moving  
        magnetOn();                         // Grab the hammer
        brakeOff();                         // Release the brake
        g_ops.seekHammer = false;           // Stop looking for the hammer
    }

    else                                    /* We need the hammer */
    if( !g_ops.seekHammer                   /* We have yet to start looking for the hammer */
    ){
        g_ops.seekHammer = true;            // Start looking for the anvil
        statusUpdate(STATUS_SEEK_HAMMER);   // Send status to the world
    }

    return g_ops.seekHammer;
}

bool seekAnvil() {

    if( g_state.anvilLimit                      /* We found the anvil */
    &&  g_ops.seekAnvil                         /* We were looking for the anvil */
    ) {
        motorOff();                             // Stop moving
        magnetOn();                             // Make sure we're still holding the hammer
        brakeOff();                             // Make sure the brake is still released
        g_ops.seekAnvil = false;                // Stop looking for the anvil
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
            motorSetSpeedAndCourse();           // Get to steppin'
        }
        
        else                                    /* We're home */ 
        {
            motorOff();                         // Stop moving
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
    &&  g_config.height < FIST_HEIGHT_MAX_INCH  /* We have a valid height setting */
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

    return g_ops.raiseHammer;
}

bool dropHammer() { /* TODO */

    return g_ops.dropHammer;
}

bool runConfiguration() {



}


int32_t m_i32LastAlarmCheck = 0; 
int32_t m_i32AlarmCheckPeriod_mSec = 5000; 
void runOperations() {

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

        if(checkHelp()) // SKIP EVERYTHING ELSE UNTIL WE GET HELP
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