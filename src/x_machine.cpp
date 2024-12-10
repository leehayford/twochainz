#include "x_machine.h"

Error ERR_HAMMERTIME_OUT("the hammer did not strike the anvil");
bool m_bHammertimeOut = false; 
hw_timer_t *tmrHammeStrike = NULL;
void setUpHammerStrikeTimer() {
    tmrHammeStrike = timerBegin(
        HAMMER_STRIKE_TIMER,
        HAMMER_STRIKE_TIMER_PRESCALE,
        HAMMER_STRIKE_TIMER_COUNT_UP
    );

    timerAttachInterrupt(
        tmrHammeStrike,
        [](){ m_bHammertimeOut = true; },
        HAMMER_STRIKE_TIMER_EDGE
    );

    timerAlarmWrite(
        tmrHammeStrike,
        HAMMER_STRIKE_TIMER_PERIOD_uSEC,
        HAMMER_STRIKE_TIMER_RUN_ONCE
    );
}

hw_timer_t *tmrOpsITR = NULL;
void IRAM_ATTR isrOpsITRTimer() {}
void setUpOpsITRTimer() {

    tmrOpsITR = timerBegin(
        OPS_ITR_TIMER,
        OPS_ITR_TIMER_PRESCALE,
        OPS_ITR_TIMER_COUNT_UP
    );

    timerAttachInterrupt(
        tmrOpsITR,
        isrOpsITRTimer,
        OPS_ITR_TIMER_EDGE
    );

    timerAlarmWrite(
        tmrOpsITR,
        OPS_ITR_TIMER_PERIOD_uSEC,
        OPS_ITR_TIMER_AUTORUN
    );

    timerAlarmEnable(tmrOpsITR);
}

void setupOps() {

    setUpOpsITRTimer();

    setUpHammerStrikeTimer();

    statusUpdate(STATUS_START);
}

void statusUpdate(const char* status_msg) {
    g_ops.setStatus(status_msg);
    Serial.printf("\nStatus: %s\n", g_ops.status);
    setMQTTPubFlag(PUB_CONFIG);
    setMQTTPubFlag(PUB_STATE);
    setMQTTPubFlag(PUB_OPS);
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
    if( err                                     /* We failed to step */
    )   g_ops.awaitHelp = true;                 // Get help    

    return err;
}

/* OPERATIONS *****************************************************************************************/

/* Called with every execution of runOperations() 
When the EStop button is pressed:
- Shut everything down
- Wait for EStop to be released 

When EStop is released
- Clear g_ops.awaitEStop
- Set g_ops.awaitHelp 

Returns true until EStop button is released */
bool awaitEStopClear() {
    
    if( g_state.eStop                           /* The emergency stop button is pressed */
    &&  !g_ops.awaitEStop                       /* We did't know the button was pressed */
    ) { 
        motorOff();                             // Stop moving
        brakeOn();                              // Apply the brake
        magnetOff();                            // Turn off the magnet
        statusUpdate(STATUS_ESTOP);             // Sing it
        g_ops.awaitEStop = true;                // We begin to yern for an enabled system
    }
       
    else                                        /* The system is enabled */      
    if( g_ops.awaitEStop                        /* We yern for an enabled system */
    ) {
        g_ops.awaitEStop = false;               // We stop yerning for an enabled system, lest we look like fools!
        g_ops.awaitHelp = true;                 // We need an operator to tell us it's ok to continue 
    }

    return g_ops.awaitEStop; 
}

/* Called with every execution of runOperations() 
When the the door is opened:
- Shut everything down
- Wait for the door to close

When the door is closed
- Clear g_ops.awaitEStop
- Set g_ops.awaitHelp 

Returns true unti door is closed */
bool awaitDoorClose() {
    
    if( g_state.doorOpen                        /* The door is open */
    &&  !g_ops.awaitDoor                        /* We did't know the door was open */
    ) { 
        motorOff();                             // Stop moving
        brakeOn();                              // Apply the brake
        magnetOff();                            // Turn off the magnet
        statusUpdate(STATUS_DOOR_OPEN);         // Sing it
        g_ops.awaitDoor = true;                 // We begin to yern for a closed door
    } 
    
    else                                        /* The door is closed */              
    if( g_ops.awaitDoor                         /* We yern for a closed door */
    ) {
        g_ops.awaitDoor = false;                // We stop yerning for a closed door, lest we look like fools!
        g_ops.awaitHelp = true;                 // We need an operator to tell us it's ok to continue          
    }

    return g_ops.awaitDoor;
}

/* Called with every execution of runOperations() 
Returns true until an operator hits reset or cancel:
MQTT message received at:
- .../cmd/ops/reset 
- .../cmd/ops/continue */
bool awaitHelp() {

    if( g_ops.awaitHelp                         /* Something bad happened and we need help */
    &&  !g_ops.seekHelp                         /* We have yet to ask for help */
    ) {
        g_ops.seekHelp = true;                  // Ask for help
        statusUpdate(STATUS_REQUEST_HELP);      // Sing it
    }

    return g_ops.awaitHelp;
}

/* Called with every execution of runOperations() 
Returns true until a valid configuration is receved */
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

/* Called by doGoHome()
Sets g_ops.seekHammer = false once we secure the hammer 
Returns an error if we fail to secure the hammer */
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

/* Called by doGoHome()
Sets g_ops.seekAnvil = false once we find the anvil 
Returns an error if we fail to find the anvil */
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

/* Called by doGoHome()  
Sets g_ops.seekHome = false once we find home 
Returns an error if we fail to find home*/
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

/* Called by runOperations() when: 
- the previous target was reached
- and g_ops.goHome is set 

Secures the hammer and places it on the anvil
Clears g_ops.goHome
Clears g_ops.recovery 
Sets g_ops.raiseHammer (if we have more cycles to complete)
Returns an error if we fail to secure the hammer secure and place it on the anvil */
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
    g_ops.goHome = false;                       // We stop going home, lest we look like fools!
    g_ops.recovery = false;                     // Clear the recovery flag (in case that's why we sought home)
    
    if( g_config.cycles > 0                     /* We have been questing */
    ) {
        if( g_ops.cycleCount < g_config.cycles  /* We have yet to finish questing */
        )   g_ops.raiseHammer = true;           // We must raise the hammer 
        // Our quest continues...

        g_config.run = false;                   // We stop questing, lest we look like fools!
        g_ops.stepTarget = 0;
        statusUpdate((char*)"DONE");
        // Our quest has ended.
    }

    return nullptr;                             // Flawless victory in any case!
    
}

/* Called by runOperations() when:
- the previous target was reached
- and g_ops.raiseHammer is set 

Raises the hammer to 'The Height of Configuration'
Clears g_ops.raiseHammer
Sets g_ops.dropHammer */
Error* doRaiseHammer() {
    g_ops.raiseHammer = false;                  // We accept this challenge
    g_ops.dropHammer = true;                    // And double-dog dare ourselves to drop the hammer next
    statusUpdate(STATUS_RAISE_HAMMER);          // We raise the hammer 
    magnetOn();                                 // With fist closed
    brakeOff();                                 // With brake off
    return moveToTarget();                      // Get to steppin'
}

/* Called by runOperations() when:
- the previous target was reached
- and g_ops.dropHammer is set 

Clears g_ops.dropHammer 
Clears hammertimeOut flag 
Enables tmrHammerStrike 
Sets g_ops.awaitStrike 
Returns an error if the time runs out */
Error* doDropHammer () {

    if( g_ops.dropHammer                        /* We have yet to drop the hammer strike */
    ) {
        g_ops.dropHammer = false;               // We accept this challenge   
        statusUpdate(STATUS_DROP_HAMMER);       // We drop the hammer 
        magnetOff();                            // With fist open
        brakeOff();                             // With brake off    

        m_bHammertimeOut = false;               // We clear the timeout flag
        timerAlarmEnable(tmrHammeStrike);       // We start the timer
        g_ops.awaitStrike = true;               /* We must await the hammer strike */
    }

    if( !m_bHammertimeOut                       /* There is still time */
    )   return nullptr;                         /* We await the hammer strike */

    /* Time has run out */
    g_ops.awaitStrike = false;                  // We stop awaiting the hammer strike, lest we look like fools!
    g_ops.dropHammer = false;                   // We stop intending to drop the hammer, lest we look like fools! 

    if( !g_ops.awaitStrike                      /* The hammer has failed to strike */
    )   return &ERR_HAMMERTIME_OUT;             // Take your shame and leave this place!
    
    /* The hammer did strike */
    g_ops.cycleCount++;                         // The hammer did strike; Count it!  
    g_ops.goHome = true;                        // We must go home
    return nullptr;                             // Flawless victory!
}


int32_t nextPosUpdate = 0;
/* Called by runOperations() when: 
- position != taget
- g_state.motorOn is set 

If position update period has passed:
- sends current position 
- schedules the next update */
void doPositionUpdate() {  

    if( millis() > nextPosUpdate                /* The time has come to publish our position */    
    ) {
        motorGetPosition();                     // Check our position
        setMQTTPubFlag(PUB_OPS_POS);            // Sing it
        nextPosUpdate =                         // Schedule the next update
            millis() + POS_UPDATE_PERIOD_mSec;    
    }
}

/* Called by main loop() 
Returns nullptr until:
- g_state.eStop is cleared 
- g_state.dooropen is cleared
- g_ops.awaitHelp is cleared 
- g_config.validate() is true  


Raises, drops and retrieves the hammer according to g_config.
Returns any error returned by:
- doGoHome()
- doRaiseHammer() 
- doDropHammer() 

Sends g_state.currentHeight periodically while fist is in motion 

Sends g_state if opsIOFlag is set */
Error* runOperations() {

    /* TODO: 
    
    Either:
        while( g_ui32InterruptFlag == 0 ) {
            ... run ops ...
        }
        handle interrupt related shite
        clear interrupt flag 

    Or:
        Use *tmrOpsITR --> isrOpsITRTimer()
    */

    if( awaitEStopClear()                       /* We yern for the system to be enabled */
    )   return nullptr;                         // We wait here thererfor...  
    
    if( awaitDoorClose()                        /* We yern for the door to close */
    )   return nullptr;                         // We wait here thererfor...  

    if( awaitHelp()                             /* We yern for assistance */
    )   return nullptr;                         // We wait here thererfor... 

    if( awaitConfig()                           /* We yern for a noble quest */
    )   return nullptr;                         // We wait here thererfor... 

    // We have work to do and the freedome to do it

    if( motorTargetReached()                    /* Our position matched our target  */
    ) {
        motorOff();                             // We stop trying to reach our target, lest we look like fools!

        if( g_ops.goHome                        /* We've been called home */
        )   return doGoHome();                  // All await our return

        if( g_ops.raiseHammer                   /* We must raise the hammer */
        )   return doRaiseHammer();             // We will rasie the hammer

        if( g_ops.dropHammer                    /* We must drop the hammer */
        )   return doDropHammer();              // We await the hammer strike 
    }

    else 
    if( g_state.motorOn                         /* We are moving */
    )   doPositionUpdate();                     // All await word of our travels

    return nullptr;

}