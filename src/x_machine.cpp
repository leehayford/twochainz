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

void doEngageRecoveryMode() {
    g_ops.recovery = true;
    g_ops.awaitHelp = true;
    statusUpdate(STATUS_RECOVERY);
}

void getRecoveryCourseAndSeed() {
    g_ops.stepTarget = MOT_REVOVERY_STEPS;      // We will do 1/4 revolution (DOWN) and reassess
    g_ops.stepHz = MOT_STEPS_PER_SEC_LOW;       // We will proceed at SLOW rip 
}

void getHomeCourseAndSeed() {                   
    g_ops.stepTarget = g_state.motorSteps * -1; // We set our course for home (DOWN)
    g_ops.stepHz = MOT_STEPS_PER_SEC_HIGH;      // We will proceed at FULL RIP! 
}

void getQuestCourseAndSpeed() {
    g_ops.stepTarget =                          // We set our course for the height of configuration (UP)
        (g_config.height / FIST_INCH_PER_REV) * MOT_STEP_PER_REV;
    g_ops.stepHz = MOT_STEPS_PER_SEC_HIGH;      // We will proceed at FULL RIP!  
}
 
Error* moveToTarget() {
   
    Error* err;

    if( g_ops.recovery                          /* We are lost and being guided home */
    )   getRecoveryCourseAndSeed();             // We will move as we are guided

    else {
        err = motorGetPosition();               // We take a bearing
        if( err                                 /* We discover we are lost */
        )   return err;                         // We will seek the guidance of a greater power 

        /* We must assume we're on track */
                                                        
        if( g_ops.goHome                        /* We've been ordered home */
        )   getHomeCourseAndSeed();             // We will make for home                     
                                                
        else                                            
        if( g_ops.raiseHammer                   /* We've been ordered to raise the hammer */
        )   getQuestCourseAndSpeed();           // We will make for the heigh of configuration
    }     
                                                                 
    motorOn();                                  // Make ready our steed! 

    err = motorSetSpeed(g_ops.stepHz);
    if( err                                     /* We have been given a warning */
    )   mqttPublishError(err);                  // We pass along the warning

    Serial.printf("\nx_machine moveToTarget:\tSTEPS: %d\tHz: %d\n", g_ops.stepTarget, g_ops.stepHz);   
    err = motorSetCourse(g_ops.stepTarget);     // Get to steppin'
    if( err                                     
    )   doEngageRecoveryMode();

    return err;
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
    &&  !g_ops.seekHelp                          /* We have yet to ask for help */
    ) {
        g_ops.seekHelp = true;                   // Ask for help
        statusUpdate(STATUS_REQUEST_HELP);      // Sing it
    }
    
    if( g_ops.seekHelp                           /* We have requested help */
    &&  !g_ops.awaitHelp                        /* We have received help */
    ) {
        g_ops.seekHelp = false;                  // Stop requesting help, lest you look like a fool!

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
            g_ops.awaitConfig = false;          // We stop awaiting configuration, lest we look like fools!
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

/* This funtion is called by doGoHome() 
Returns an error if we fail to secure the hammer 
Sets g_ops.seekHammer = false once we secure the hammer */
Error* doSeekHammer() { 

    if( g_state.fistLimit                       /* We found the hammer */
    ) {
        if( g_ops.seekHammer                    /* We were seeking the hammer */
        )   motorOff();                         // Stop moving while we secure the hammer 
        
        magnetOn();                             // Secure the hammer
        g_ops.seekHammer = false;               // Stop seeking the hammer, lest you look like a fool!
        return nullptr;                         // Take the hammer and leave this place! 
        // Our search has ended.
    }
    
    if( !g_ops.seekHammer                       /* We have yet to seek the hammer */
    )   statusUpdate(STATUS_SEEK_HAMMER);       // Sing it
        // Our search begins!

    g_ops.seekHammer = true;                    // We seek the hammer
    magnetOff();                                // With fist open
    brakeOn();                                  // With brake on
    return moveToTarget();                       // Get to steppin'
    // Our search continues...
}

/* This funtion is called by doGoHome() 
Returns an error if we fail to find the anvil 
Sets g_ops.seekAnvil = false once we find the anvil */
Error* doSeekAnvil() {

    if( g_state.anvilLimit                      /* We found the anvil */
    ) {
        g_ops.seekAnvil = false;                // Stop seeking the anvil, lest you look like a fool!
        return nullptr;                         // Take the anvil and leave this place... no wait that's heavy; I'll go; the anvil is your now... I... don't really know what I'm... I just ummm, don't have anywh- Hey! where are you going? No. Sorry. You've got you own thing. You know what, I'm sure I'll figure it out; don't worry about it...
        // Our search has ended.
    }

    if( !g_ops.seekAnvil                        /* We have yet to seek the anvil */
    )   statusUpdate(STATUS_SEEK_ANVIL);        // Sing it
        // Our search begins!

    g_ops.seekAnvil = true;                     // We seek the anvil
    magnetOn();                                 // With fist closed
    brakeOff();                                 // With brake off
    return moveToTarget();                       // Get to steppin'
    // Our search continues...
}

/* This funtion is called by doGoHome() 
Returns an error if we fail to find home 
Sets g_ops.seekHome = false once we find home */
Error* doSeekHome() {
        
    if( g_state.homeLimit                       /* We found home */
    ) {
        g_ops.seekHome = false;                 // Stop seeking home, lest you look like a fool!
        return nullptr;                         //      
        // Our search has ended.
    }

    if( !g_ops.seekHome                         /* We have yet to seek home */
    )   statusUpdate(STATUS_SEEK_HOME);         // Sing it
        // Our search begins!

    g_ops.seekHome = true;                      // We seek home
    magnetOn();                                 // With fist closed
    brakeOff();                                 // With brake off
    return moveToTarget();                       // Get to steppin'
    // Our search continues...
}

/* Call this funtion to seek home 
Returns an error if we fail to return home with the hammer 
sets g_ops.goHome = false once we return home with the hammer */
Error* doGoHome() {
    Error* err;

    err = doSeekHammer();
    if( err                                     /* We have failed to seek the hammer */
    )   return err;                             // We make our failure known

    if( g_ops.seekHammer                        /* We continue to seek the hammer */
    )   return nullptr;                         // We go no further without the hammer

    // With the hammer secure in our fist, we seek the anvil

    err = doSeekAnvil();
    if( err                                     /* We have failed to seek the anvil */
    )   return err;                             // We make our failure known
    
    if( g_ops.seekAnvil                         /* We continue to seek the anvil */
    )   return nullptr;                         // We go no further without the anvil
    
    // With the hammer secure and the anvil found, we seek home 

    err = doSeekHome();
    if( err                                     /* We have failed to seek home */
    )   return err;                             // We make our failure known
    
    if( g_ops.seekHome                          /* We continue to seek home */
    )   return nullptr;                         // We go no further without finding home

    // We are home, with hammer secured, and anvil found

    motorOff();                                 // Stop moving
    motorSetPositionAsZero();                   // Reset our home position
    g_ops.recovery = false;                     // Clear the recovery flag (in case that's why we sought home)
    g_ops.goHome = false;                       // Stop going home
    return nullptr;
    // This leg of our quest has ended.
}



Error* doRaiseHammer() {
    g_ops.raiseHammer = true;                   // We raise the hammer
    magnetOn();                                 // With fist closed
    brakeOff();                                 // With brake off
    statusUpdate(STATUS_RAISE_HAMMER);          // Sing it
    return moveToTarget();                      // Get to steppin'
}

Error* doDropHammer () {
    g_ops.dropHammer = true;                    // We drop the hammer 
    magnetOff();                                // With fist open
    brakeOff();                                 // With brake off    
    statusUpdate(STATUS_DROP_HAMMER);           // Sing it 
    return nullptr;   
}

//err = &ERR_HAMMERTIME_OUT 

Error ERR_HAMMERTIME_OUT("the hammer did not strike the anvil");
Error* doAwaitHammerStrike() {

    Error* err = nullptr;

    /* TODO:   
    if( HAMMERTIME_OUT
    )   
    */ 
   
    if( err                                     /* We await too long */
    ||  g_state.anvilLimit                      /* The hammer did strike */
    ) {
        /* TODO: 
            DISABLE HAMMERTIME_OUT */
    }

    return err;  
}

Error* runConfiguration() { 

    Error* err;

    if( motorTargetReached()                    /* Our position matched our target  */
    ) {
        motorOff();                             // We stop trying to reach our target, lest we look like fools!

        if( g_ops.stepTarget == 0               /* We sit at home having yet to begin... like a dullard! */
        )   return doRaiseHammer();             // We raise the hammer for the first time
        // Our quest begins...

        if( g_ops.raiseHammer                   /* We've raised the hammer */
        ) {
            g_ops.raiseHammer = false;          // We stop intending to raise the hammer, lest we look like a fool!
            return doDropHammer();              // We drop the hammer; the time for action has come
        }

        if( g_ops.dropHammer                    /* We've dropped the hammer */
        ) {
            g_ops.dropHammer = false;           // We stop intending to drop the hammer, lest we look like a fool!
            g_ops.awaitDrop = true;             /* We must await the hammer strike */
            /* TODO: 
                ENABLE HAMMERTIME_OUT */
        }

        if( g_ops.awaitDrop
        ) {
            err = doAwaitHammerStrike();
            if( err                             /* The hammer has failed to strike */
            )   return err;
   
            if( !g_state.anvilLimit             /* We await the hammer strike */
            )   return nullptr;
 
            g_ops.cycleCount++;                 // The hammer did strike; Count it!
            g_ops.awaitDrop = false;            // We stop awaiting the hammer strike, lest we look like fools!
            g_ops.goHome = true;                // We must go home
        }



        if( g_ops.goHome
        ) {
            err = doGoHome();
            if( err                             /* We have failed to return home with the hammer */
            )   return err;
            
            if( g_ops.seekHome                  /* We continue to return home with the hammer */
            )   return nullptr;                          

            // We have secured the hammer, found the anvil and arrived at home

            if( g_ops.cycleCount == g_config.cycles     /* We have completed out quest */
            ) { 
                g_config.run = false;           // Stop questing, lest we look like a fool!
                g_ops.stepTarget = 0;
                statusUpdate((char*)"DONE");
                return nullptr;
                // Our quest has ended.
            }

            return doRaiseHammer();             // We raise the hammer again
            // Our quest continues...
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
            
    if( awaitEStopClear()                       /* We yern for the emergency stop button to be released */
    )   return nullptr;                         // We wait here thererfor...  
    
    // SYSTEM IS ENABLED; CONTINUE
    
    if( awaitDoorClose()                        /* We yern for the door to close */
    )   return nullptr;                         // We wait here thererfor...  
    
    // DOOR IS CLOSED; CONTINUE

    if( awaitHelp()                             /* We yern for assistance */
    )   return nullptr;                         // We wait here thererfor... 
    
    // WE FACE OUR FATE ALONE; CONTINUE

    if( awaitConfig()                           /* We yern for a noble quest */
    )   return nullptr;                         // We wait here thererfor... 

    // WE HAVE OUR QUEST; CONTINUE 

    err = runConfiguration(); // RUN CONFIGURATION 
    if( err
    )   return err;

    // Check if we should send a motor position update
    if( millis() > m_i32NextPositionUpdate      /* We must publish our position */    
    ) {
        motorGetPosition();                     // Check our position
        setMQTTPubFlag(PUB_OPS_POS);            // Sing it
        m_i32NextPositionUpdate =               // Schedule the next position update
            millis() + m_i32PositionUpdatePeriod_mSec;    
    }

    return nullptr;

}