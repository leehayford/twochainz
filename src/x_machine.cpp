#include "x_machine.h"

Error* m_pErr;

void statusUpdate(const char* status_msg) {
    g_ops.setStatus(status_msg);
    Serial.printf("\nStatus: %s\n", g_ops.status);
    setMQTTPubFlag(PUB_CONFIG);
    setMQTTPubFlag(PUB_STATE);
    setMQTTPubFlag(PUB_OPS);
}

int32_t m_i32NextPositionUpdate = 0;
int32_t m_i32PositionUpdatePeriod_mSec = 300;

// uint32_t m_ui32MotorTargetSteps = 0;
// uint32_t m_ui32MotorSpeed = MOT_STEPS_PER_SEC_HIGH;

Error ERR_MOT_TARGET_ZERO("motor target steps must not be zero");
Error* motorSetCourseAndSpeed() {
   
    if( !motorCheckPosition()                           /* We're lost */
    ||  g_ops.recovery                                  /* We're lost and reassissing */
    ) {  
        if( !g_ops.recovery                             /* We didn't know we were lost */
        ) {   
            g_ops.awaitHelp = true;                     // We need an operator to tell us it's ok to continue            
            g_ops.recovery = true;                      // Come back and reassess until we've been home
            statusUpdate(STATUS_RECOVERY);              // Sing it
        }
        
        g_ops.stepTarget = MOT_REVOVERY_STEPS;          // Do 1/4 revolution (DOWN) and reassess
        g_ops.stepHz = MOT_STEPS_PER_SEC_LOW;           // Set speed to *** SLOW ***
    }

    else                                                /* We assume we're on track */
    {   
        // Serial.printf("\nSetCourseAndSpeed: ON TRAK \n");                                                
        if( g_ops.awaitHammer                           /* We need to go DOWN */
        ||  g_ops.awaitAnvil                            /* We need to go DOWN */
        ||  g_ops.goHome                                /* We need to go DOWN */
        ) {  
            g_ops.stepTarget =                          // Go DOWN 
                g_state.motorSteps * -1;                // To zero height
            // Serial.printf("\nSetCourseAndSpeed: GO HOME : %d\n", g_ops.stepTarget); 
        }

        else                                            
        if( g_ops.raiseHammer                           /* We need to go UP */
        ) {   
            g_ops.stepTarget =                          // Go UP 
                (g_config.height / FIST_INCH_PER_REV)   // To configured height
                * MOT_STEP_PER_REV;      
            // Serial.printf("\nSetCourseAndSpeed: RAISE HAMMER : %d\n", g_ops.stepTarget);       
        } 

        g_ops.stepHz = MOT_STEPS_PER_SEC_HIGH;          // Set speed to *** FULL RIP ***                  
    }

    if( g_ops.stepTarget == 0                           /* We can't step to that */
    ) {  
        g_ops.recovery = true;
        return &ERR_MOT_TARGET_ZERO;
    }

    motorOn();                                                    
    motorMoveRelativeSteps(                         // Get to steppin' 
        g_ops.stepTarget,                           // This far
        g_ops.stepHz                                // This fast
    );
    return nullptr;

}

/* OPERATIONS *****************************************************************************************/

/* When the EStop button is pressed:
    - Shut everything down
    - Wait for EStop to be released 

When EStop is released
    - Go home in recovery mode 
    
Returns true until EStop button is released */
bool awaitEStopClear() {
    
    if( g_state.eStop                           /* The emergency stop button is pressed */
    &&  !g_ops.awaitEStop                       /* We did't know the button was pressed */
    ) { 
        motorOff();                             // Stop moving
        brakeOn();                              // Apply the brake
        magnetOff();                            // Turn off the magnet
        g_ops.awaitEStop = true;                // Wait for the button to be reset
        statusUpdate(STATUS_ESTOP);             // Sing it
    }
       
    else                                        
    if( !g_state.eStop                          /* The system has been enabled */
    &&  g_ops.awaitEStop                        /* We have yet to clear the flag */
    ) {
        g_ops.awaitEStop = false;               // Stop waiting for the system to be enabled
        g_ops.awaitHelp = true;                 // We need an operator to tell us it's ok to continue 
    }

    return g_ops.awaitEStop; 
}

/* When the the door is opened:
    - Shut everything down
    - Wait for the door to close

When the door is closed
    - Go home in recovery mode 

Returns true unti door is closed */
bool awaitDoorClose() {
    
    if( g_state.doorOpen                        /* The door is open */
    &&  !g_ops.awaitDoor                        /* We did't know the door was open */
    ) { 
        motorOff();                             // Stop moving
        brakeOn();                              // Apply the brake
        magnetOff();                            // Turn off the magnet
        g_ops.awaitDoor = true;                 // Wait for the door to close
        statusUpdate(STATUS_DOOR_OPEN);         // Sing it
    } 
    
    else                                        
    if( !g_state.doorOpen                       /* The door is closed */
    &&  g_ops.awaitDoor                         /* We have yet to clear the flag */
    ) {
        g_ops.awaitDoor = false;                // Stop waiting for the door to close
        g_ops.awaitHelp = true;                 // We need an operator to tell us it's ok to continue          
    }

    return g_ops.awaitDoor;
}

/* Returns true until an operator hits reset or cancel:
    MQTT message received at:
    - .../cmd/ops/reset 
    - .../cmd/ops/continue */
bool awaitHelp() {

    if( g_ops.awaitHelp                         /* Something bad happened and we need help */
    &&  !g_ops.reqHelp                          /* We have yet to ask for help */
    ) {
        g_ops.reqHelp = true;                   // Ask for help
        statusUpdate(STATUS_REQUEST_HELP);      // Sing it
    }
    
    if( g_ops.reqHelp                           /* We have requested help */
    &&  !g_ops.awaitHelp                        /* We have received help */
    ) {
        g_ops.reqHelp = false;                  // Stop requesting help

        // These flags will be cleared in the doGoHome function
        g_ops.recovery = true;                  // We must be cautious 
        g_ops.goHome = true;                    // We must go home 
    }

    return g_ops.awaitHelp;
}

/* Returns true until a valid configuration is receved */
bool awaitConfig() {

    if( g_config.validate()                     /* We have a valid configuration */
    ) {
        if( g_ops.awaitConfig                   /* We were waiting for a valid configuration */
        ) {
            g_ops.clearProgress();              // Prepare to start a new campaign 
            g_ops.awaitConfig = false;          // Stop waiting for a configuration
            g_ops.awaitStart = true;            // We must check some things before we begin
        }
    }

    else                                        /* We must wait for a valid configuration */
    if( !g_ops.awaitConfig                      /* We have yet to start waiting */
    ){
        g_ops.awaitConfig = true;               // Start waiting
        statusUpdate(STATUS_AWAIT_CONFIG);      // Sing it
    } 

    return g_ops.awaitConfig;
}


Error* doRaiseHammer() {
    g_ops.raiseHammer = true;                   // We raise the hammer
    magnetOn();                                 // With fist closed
    brakeOff();                                 // With brake off
    statusUpdate(STATUS_RAISE_HAMMER);          // Sing it
    return motorSetCourseAndSpeed();            // Get to steppin'
}

void doDropHammer() {
    g_ops.dropHammer = true;                    // We drop the hammer 
    magnetOff();                                // With fist open
    brakeOff();                                 // With brake off                 
    /* TODO: START HAMMERTIME_OUT */ g_ops.awaitDrop = true;
    statusUpdate(STATUS_DROP_HAMMER);           // Sing it       
}

/* This funtion is called by calling doGoHome() 
Returns true until we have secured the hammer */
sboolErr seekHammer() { 

    if( g_state.fistLimit                           /* We found the hammer */
    ) {
        if( g_ops.awaitHammer                       /* We were seeking the hammer */
        )   motorOff();                             // Stop moving while we secure the hammer 
        
        magnetOn();                                 // Secure the hammer
        g_ops.awaitHammer = false;                  // Stop seeking the hammer
        return {                                    // Take the hammer and leave this place!
            g_ops.awaitHammer,
            nullptr
        };  // Our Quest has ended.
    }
    
    if( !g_ops.awaitHammer                          /* We have yet to seek the hammer */
    )   statusUpdate(STATUS_SEEK_HAMMER);           // Sing it
        // Our quest begins!

    g_ops.awaitHammer = true;                       // We seek the hammer
    magnetOff();                                    // With fist open
    brakeOn();                                      // With brake on
    return {                                        // Get to steppin'
        g_ops.awaitHammer, 
        motorSetCourseAndSpeed()
    };  // Our quest continues...
}

/* This funtion is called by calling doGoHome() 
Returns true until we have found the anvil */
sboolErr seekAnvil() {

    if( g_state.anvilLimit                          /* We found the anvil */
    ) {
        g_ops.awaitAnvil = false;                   // Stop seeking the anvil
        return {                                    // Take the anvil and leave this place... no wait that's heavy; I'll go; the anvil is your now... I... don't really know what I'm... I just ummm, don't have anywh- Hey! where are you going? No. Sorry. You've got you own thing. You know what, I'm sure I'll figure it out; don't worry about it...
            g_ops.awaitAnvil,
            nullptr
        };  // Our Quest has ended.
    }

    if( !g_ops.awaitAnvil                           /* We have yet to seek the anvil */
    )   statusUpdate(STATUS_SEEK_ANVIL);            // Sing it
        // Our quest begins!

    g_ops.awaitAnvil = true;                        // We seek the anvil
    magnetOn();                                     // With fist closed
    brakeOff();                                     // With brake off
    return {                                        // Get to steppin'
        g_ops.awaitAnvil, 
        motorSetCourseAndSpeed()
    };  // Our quest continues...
}

/* Call this funtion to seek home 
Returns true until we are at home */
sboolErr doGoHome() {
    sboolErr bErr;

    bErr = seekHammer();
    if( bErr.err                                /* We have failed to seek the hammer */
    ||  bErr.bRes                               /* We continue to seek the hammer */
    )   return bErr;

    bErr = seekAnvil();
    if( bErr.err                                /* We have failed to seek the anvil */
    ||  bErr.bRes                               /* We continue to seek the anvil */
    )   return bErr;                
    
    // With both hammer and anvil, we may return home 

    if( g_state.homeLimit                       /* We found home */
    ) {
        motorOff();                             // Stop moving
        motorSetPositionAsZero();               // Reset our home position
        g_ops.recovery = false;                 // Clear the recovery flag (in case that's why we sought home)
        g_ops.goHome = false;                   // Stop going home
        return {
            g_ops.goHome,
            nullptr
        };// Our Quest has ended.
    }

    if( !g_ops.goHome                           /* We have yet to seek home */
    )   statusUpdate(STATUS_SEEK_HOME);         // Sing it
        // Our quest begins!
    

    g_ops.goHome = true;                        // We seek home
    magnetOn();                                 // With fist closed
    brakeOff();                                 // With brake off 
    return {                                    // Get to steppin'
        g_ops.goHome, 
        motorSetCourseAndSpeed()
    };  // Our quest continues...
}


bool doSafeStartCheck() { /* TODO: General safety checks */

    if( g_ops.awaitStart 
    ) {
        // if( doGoHome()                                      /* We are traveling home */
        // ) { 
        //     g_ops.awaitStart = true;
        //     return g_ops.awaitStart;
        // }

        /* TODO: 
        Other general safety checks... 
        */

        /* We are ready to start */  
        doRaiseHammer();
        g_ops.awaitStart = false;
    }  
    
    return g_ops.awaitStart;
}


Error* runConfiguration() { 

    sboolErr bErr;

    if( motorTargetReached()) {

        motorOff();                                     // Stop trying to reach the target... You look like a fool!

        if( g_ops.dropHammer                            /* We've have dropped the hammer */
        ) {
            /* TODO: HAMMERTIME_OUT */ g_ops.awaitDrop = false;
            
            if( g_ops.awaitDrop                         /* The hammer has yet to fall */
            )   return nullptr;                         /* TODO: HAMMERTIME_OUT */

            // The hammer has dropped
            g_ops.cycleCount++;                         // Count it

            bErr = doGoHome();
            if( bErr.err                                /* We have failed to retrieve the hammer */
            )   return bErr.err;
            
            if( bErr.bRes                               /* We continue to retrieve the hammer */
            )   return nullptr;

            // We have returned home with the hammer

            g_ops.dropHammer = false;                   // Stop intending to drop the hammer

            
            if( g_ops.cycleCount == g_config.cycles     /* We have completed out quest */
            ) { 
                g_config.run = false;                   // Stop questing
                g_ops.stepTarget = 0;
                statusUpdate((char*)"DONE");
                return nullptr;
            }

            return doRaiseHammer();                     // Continue questing
        }

        if( g_ops.raiseHammer                           /* We've raised the hammer */
        ) {
            g_ops.raiseHammer = false;                  // Stop intending to raise the hammer
            doDropHammer();                             // Drop the hammer
        }

    }

    return nullptr;

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
Error* runOperations() {
    Error* err;
    // If we haven't sent state updated in a while, do it 
    // if(g_ui32InterruptFlag == 0 && m_i32LastAlarmCheck + m_i32AlarmCheckPeriod_mSec < millis()) {
    //     checkAllITRPins();
    //     setMQTTPubFlag(PUB_STATE);
    //     g_ui32InterruptFlag = 1;
    // }

    if (g_ui32InterruptFlag > 0) { 
        m_i32LastAlarmCheck = millis();
        g_ui32InterruptFlag = 0;
        setMQTTPubFlag(PUB_STATE);
    }
            
    // if(awaitEStopClear()) // SKIP EVERYTHING ELSE UNTIL SYSTEM IS ENABLED  
    //     return; 
    // // SYSTEM IS ENABLED; CONTINUE
    

    // if(awaitDoorClose()) // SKIP EVERYTHING ELSE UNTIL DOOR IS CLOSED 
    //     return; 
    // // DOOR IS CLOSED; CONTINUE


    // if(awaitHelp()) // SKIP EVERYTHING ELSE UNTIL WE GET HELP
    //     return;
    // // WE HAVE BEEN HELPED; CONTINUE


    if(awaitConfig()) // SKIP EVERYTHING ELSE UNTIL WE ARE CONFIGURED TO RUN 
        return nullptr;
    // CONFIGURED TO RUN; CONTINUE 


    if(doSafeStartCheck()) // SKIP EVERYTHING ELSE UNTIL IT IS SAFE TO START OPERATIONS  
        return nullptr;
    // SAFE TO START OPERATIONS; CONTINUE


    err = runConfiguration(); // RUN CONFIGURATION 
    if( err
    )   return err;

    // if (motorTargetReached()) {

    //     motorOff();                                         // Stop trying to reach the target... You look like a fool!

    //     if( g_ops.stepTarget == 0                           // We are starting the fist cycle
    //     ) { 
    //         motorSetPositionAsZero();
    //         g_ops.raiseHammer = true;                       // Start intending to raise the hammer
    //         g_ops.goHome = false;                           // Stop intending to go home
    //         statusUpdate((char*)"BEGIN...");
    //     } 

    //     if( g_ops.stepTarget < 0                            // We have just returned from the drop height
    //     ) {                    
    //         g_ops.cycleCount++;                             

    //         if( g_ops.cycleCount == g_config.cycles         // We are done
    //         ) { 
    //             g_config.run = false;                       // Stop doing things
    //             g_ops.stepTarget = 0;
    //             statusUpdate((char*)"DONE");
    //             return;
    //         }

    //         doRaiseHammer();
    //     }

    //     else { // We're at the top; DROP THE HAMMER!... and then go get it.
    //         doDropHammer();
    //     }

    //     delay(100); // Serial.printf("\nm_steps: %d", m_i32StepsTarget);
    //     motorSetCourseAndSpeed();                   // Get to steppin'

    // } 

    // Check if we should send a motor position update
    if( millis() > m_i32NextPositionUpdate              /* We must publish our position */    
    ) {
        motorCheckPosition();                           // Check our position
        setMQTTPubFlag(PUB_OPS_POS);                    // Sing it
        m_i32NextPositionUpdate =                       // Schedule the next position update
            millis()              
            + m_i32PositionUpdatePeriod_mSec;    

    }

    return nullptr;

}