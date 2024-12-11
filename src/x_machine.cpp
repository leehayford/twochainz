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
void IRAM_ATTR isrOpsITRTimer() {
    if( g_ui32InterruptFlag                     /* We have had a state change */
    ) {
        setMQTTPubFlag(PUB_STATE);              // We tell all of our state change
        g_ui32InterruptFlag = 0;                // We stop reacting to the state change, lest we look like fools!
    }
}
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
    
    // Serial.printf("\nx_machine getQuestCourseAndSpeed:\tSTEPS: %d\tHz: %d\n", g_ops.stepTarget, g_ops.stepHz); 
}
 
Error* moveToTarget() {
   
    Error* err = nullptr;

    if( g_ops.reorient                          /* We are lost and being guided home */
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

    if( motorTargetReached()
    ) {
        motorOn();                             // Make ready our steed! 

        err = motorSetSpeed(g_ops.stepHz);
        if( err                                     /* We have been given a warning */
        )   mqttPublishError(err);                  // We pass along the warning

        Serial.printf("\nx_machine moveToTarget:\tSTEPS: %d\tHz: %d\n", g_ops.stepTarget, g_ops.stepHz);   
        err = motorSetCourse(g_ops.stepTarget);     // We get to steppin'
        if( err                                     /* We have failed to step */
        )   g_ops.wantAid = true;                   // We yearn for aid 
    }
   
    return err;
}

/* OPERATIONS *****************************************************************************************/

/* Called with every execution of runOperations() 
When the EStop button is pressed:
- Shut everything down
- Wait for EStop to be released 

When EStop is released
- Clear g_ops.wantEStopRelease
- Set g_ops.wantAid 

Returns true until EStop button is released */
bool isEStopPressed() {
    
    if( g_state.eStop                           /* The emergency stop button is pressed */
    ) {
        motorOff();                             // We stop moving
        brakeOn();                              // We apply the brake
        magnetOff();                            // We turn off the magnet

        if( !g_ops.wantEStopRelease             /* We were unaware of this pressedness */
        )   statusUpdate(STATUS_ESTOP);         // We expose this treachery!

        g_ops.wantEStopRelease = true;          // We yearn for release
    }
       
    else                                        /* Release has been given */   
    if( g_ops.wantEStopRelease                  /* Still we yearn for release */
    ) {
        g_ops.wantEStopRelease = false;         // We stop yearning for release, lest we look like perverts!
        g_ops.wantAid = true;                   // We begin yearning for the aid of an operator
    }

    return g_ops.wantEStopRelease; 
}

/* Called with every execution of runOperations() 
When the the door is opened:
- Shut everything down
- Wait for the door to close

When the door is closed
- Clear g_ops.wantEStopRelease
- Set g_ops.wantAid 

Returns true unti door is closed */
bool isDoorOpen() {
    
    if( g_state.doorOpen                        /* The door is open */
    ) {
        motorOff();                             // Stop moving
        brakeOn();                              // Apply the brake
        magnetOff();                            // Turn off the magnet

        if( !g_ops.wantDoorClose                /* We were unaware of this openness */
        )   statusUpdate(STATUS_DOOR_OPEN);     // We expose this treachery!

        g_ops.wantDoorClose = true;             // We yearn for closure
    } 
    
    else                                        /* The door is closed */  
    if( g_ops.wantDoorClose                     /* Still we yearn for closure */
    ) {
        g_ops.wantDoorClose = false;            // We stop yearning for closure, lest we appear incapable of emotional growth!
        g_ops.wantAid = true;                   // We begin yearning for the aid of an operator        
    }

    return g_ops.wantDoorClose;
}

/* Called with every execution of runOperations() 
Returns true until an operator hits reset or cancel:
MQTT message received at:
- .../cmd/ops/reset 
- .../cmd/ops/continue */
bool isAidRequired() {

    if( g_ops.wantAid                           /* Something bad happened, thus we yearn for aid */
    ) {
        if( !g_ops.requestAid                   /* We have yet to call for aid */
        )   statusUpdate(STATUS_REQUEST_HELP);  // We swallow our pride; no Chainz is an island; TwoChainz, doubly so...
        
        g_ops.requestAid = true;                // We will not ask again; it was hard enough the first time...
    }

    return g_ops.wantAid;
}

/* Called with every execution of runOperations() 
Returns true until a valid configuration is receved */
bool isConfigRequired() {

    if( !g_config.validate()                    /* We lack clear attainable goals */
    ) {
        if( !g_ops.wantConfig                   /* We were unaware of this personal failing */
        )   statusUpdate(STATUS_WANT_CONFIG);   // We fall to our knees, arms wide, fists clenched, and shout to he heavens, "WHAT DO YOU WANT FROM US!?"

        g_ops.wantConfig = true;                // We yearn for purpose
    }

    else                                        /* We are charged with glorious purpose */
    if( g_ops.wantConfig                        /* Still we yearn for purpose */
    ) {
        g_ops.wantConfig = false;               // We stop yearning for purpose, lest we appear ungrateful!
        g_ops.clearProgress();                  // To hell with all that has come before! Our singular concern is this moment, this quest. Onward! To glory! Or to ruin!  
    }

    return g_ops.wantConfig;
}

/* Called by doGoHome()
Sets g_ops.seekHammer = false once we secure the hammer 
Returns an error if we fail to secure the hammer */
void doSeekHammer() { 

    if( g_state.fistLimit                       /* We found the hammer */
    ) { 
        motorOff();                             // Stop moving while we secure the hammer 
        magnetOn();                             // Secure the hammer
        g_ops.seekHammer = false;               // Stop seeking the hammer, lest you look like a fool!
        return;                                 // Take the hammer and leave this place! 
        // Our search has ended.
    }
    
    if( !g_ops.seekHammer                       /* We have yet to seek the hammer */
    )   statusUpdate(STATUS_SEEK_HAMMER);       // Sing it
        // Our search begins!

    g_ops.seekHammer = true;                    // We seek the hammer
    motorOn();                                  // With motor on
    magnetOff();                                // With fist open
    brakeOn();                                  // With brake on
    // Our search continues...
}

/* Called by doGoHome()
Sets g_ops.seekAnvil = false once we find the anvil 
Returns an error if we fail to find the anvil */
void doSeekAnvil() {

    if( g_state.anvilLimit                      /* We found the anvil */
    ) {
        g_ops.seekAnvil = false;                // Stop seeking the anvil, lest you look like a fool!
        return;                                 // Take the anvil and leave this place... no wait that's heavy; I'll go; the anvil is yours now... I... don't really know what I'm... I just ummm, don't have anywh- Hey! where are you going? No. Sorry. You've got you own thing. You know what, I'm sure I'll figure it out; don't worry about it...
        // Our search has ended.
    }

    if( !g_ops.seekAnvil                        /* We have yet to seek the anvil */
    )   statusUpdate(STATUS_SEEK_ANVIL);        // Sing it
        // Our search begins!

    g_ops.seekAnvil = true;                     // We seek the anvil
    motorOn();                                  // With motor on
    magnetOn();                                 // With fist closed
    brakeOff();                                 // With brake off
    // Our search continues...
}

/* Called by doGoHome()  
Sets g_ops.seekHome = false once we find home 
Returns an error if we fail to find home*/
void doSeekHome() {
        
    if( g_state.homeLimit                       /* We found home */
    ) {
        g_ops.seekHome = false;                 // Stop seeking home, lest you look like a fool!
        return;                                 //      
        // Our search has ended.
    }

    if( !g_ops.seekHome                         /* We have yet to seek home */
    )   statusUpdate(STATUS_SEEK_HOME);         // Sing it
        // Our search begins!

    g_ops.seekHome = true;                      // We seek home
    motorOn();                                  // With motor on
    magnetOn();                                 // With fist closed
    brakeOff();                                 // With brake off
    // Our search continues...
}

/* Called by runOperations() when: 
- the previous target was reached
- and g_ops.goHome is set 

Secures the hammer and places it on the anvil
Clears g_ops.goHome
Clears g_ops.reorient 
Sets g_ops.raiseHammer (if we have more cycles to complete)
Returns an error if we fail to secure the hammer secure and place it on the anvil */
Error* doGoHome() {
    
    bool move = false;

    if( g_state.fistLimit
    &&  g_state.anvilLimit
    &&  g_state.homeLimit
    )   {
        motorOff();                                 // Stop moving
        motorSetPositionAsZero();                   // Reset our home position
        g_ops.goHome = false;                       // We stop going home, lest we look like fools!
        g_ops.reorient = false;                     // Clear the reorient flag (in case that's why we sought home)
        
        if( g_config.cycles > 0                     /* We have been questing */
        &&  g_ops.cycleCount < g_config.cycles      /* We have yet to finish questing */
        )   g_ops.raiseHammer = true;               // We must raise the hammer 
            // Our quest continues...

        else {
            g_config.run = false;                   // We stop seeking either glory or ruin, lest we look like fools!
            g_ops.stepTarget = 0;                   // We have taken every step
            statusUpdate((char*)"DONE");
            // Our quest has ended.
        }
        return nullptr;                             // Flawless victory in any case!
    }

    if( g_ops.reorient                              /* We are being guided home */
    ||  (   motorTargetReached()                    /* We have yet to begin going home */
        &&  g_state.motorSteps != 0                 /* We are not at home */
        )                                        
    ) {
        Error* err = moveToTarget();                // We get to steppin'
        if( err                                     /* We have failed to do the steppin' */
        )   return err;                             // We make our failure known
    } 

    if( !g_state.fistLimit                          /* We yearn for the hammer */
    )   doSeekHammer();                              

    else                                            /* With the hammer secure in our fist */
    if( !g_state.anvilLimit                         /* We yearn for the anvil */
    )   doSeekAnvil();                              

    else                                            /* With the hammer secure and the anvil found */
    if( !g_state.homeLimit                          /* We yearn for home */
    )   doSeekHome();                                

    return nullptr;
}

/* Called by runOperations() when:
- g_ops.raiseHammer is set 

Raises the hammer to 'The Height of Configuration'
Clears g_ops.raiseHammer
Sets g_ops.dropHammer */
Error* doRaiseHammer() {

    if( g_state.motorSteps == 0                 /* We languish at home as glory beckons */             
    ) {    
        statusUpdate(STATUS_RAISE_HAMMER);      // We raise the hammer 
        magnetOn();                             // With fist closed
        brakeOff();                             // With brake off
        return moveToTarget();                  // And we get to steppin'
    }

    if( motorTargetReached()                    /* We ahve raised the hammer */
    ) {
        g_ops.raiseHammer = false;              // We have met this challenge
        g_ops.dropHammer = true;                // And now double-dog-dare ourselves to drop the hammer 
    }
    return nullptr;
}

/* Called by runOperations() when:
- the previous target was reached
- and g_ops.dropHammer is set 

Clears g_ops.dropHammer 
Clears hammertimeOut flag 
Enables tmrHammerStrike 
Sets g_ops.wantStrike 
Returns an error if the time runs out */
Error* doDropHammer () {

    if( !g_ops.wantStrike                       /* We have yet to drop the hammer */
    ) { 
        statusUpdate(STATUS_DROP_HAMMER);       // We drop the hammer 
        magnetOff();                            // With fist open
        brakeOff();                             // With brake off    
        m_bHammertimeOut = false;               // With timeout flag cleared
        timerAlarmEnable(tmrHammeStrike);       // With hammertime upon us

        g_ops.wantStrike = true;                /* And we yearn for the strike of the hammer */
    }

    if( !m_bHammertimeOut                       /* There is still time */
    )   return nullptr;                         /* We await the hammer strike */

    /* Time has run out */
 
    g_ops.wantStrike = false;                   // We stop yearning for the hammer strike, lest we look like fools!
    g_ops.dropHammer = false;                   // We have already answered this challenge 

    if( !g_state.anvilLimit                     /* The hammer has failed to strike */
    ) {                                      
       g_ops.wantAid;                           // We yearn for aid
       return &ERR_HAMMERTIME_OUT;              // We feel shame and confusion
    }
     
    g_ops.cycleCount++;                         // Count it!
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
- g_ops.wantEStopRelease is cleared 
- g_ops.wantDoorClose is cleared
- g_ops.wantAid is cleared 
- g_ops.wantConfig is cleared  

Raises, drops and retrieves the hammer according to: 
- g_config.cycles
- g_config.height

Passes along any error returned by:
- doGoHome()
- doRaiseHammer() 
- doDropHammer() 

Disables motor when target is reached

Periodically publishes position if fist is in motion */
Error* runOperations() {

    if( isEStopPressed()                        /* We yearn for the system to be enabled */
    )   return nullptr;                         // We go no further...  
    
    if( isDoorOpen()                            /* We yearn for the door to close */
    )   return nullptr;                         // We go no further... 

    if( isAidRequired()                         /* We yearn for assistance */
    )   return nullptr;                         // We go no further... 

    if( isConfigRequired()                      /* We yearn for purpose */
    )   return nullptr;                         // We go no further...  

    // We have work to do and the freedome to do it

    if( g_state.motorOn                         /* We are moving */
    )   doPositionUpdate();                     // All await word of our travels

    if( g_ops.goHome                            /* We've been called home */
    )   return doGoHome();                      // All await our return

    if( g_ops.raiseHammer                       /* We must raise the hammer */
    )   return doRaiseHammer();                 // We will rasie the hammer

    if( g_ops.dropHammer                        /* We must drop the hammer */
    )   return doDropHammer();                  // We await the hammer strike 
    
    // if( motorTargetReached()                    /* Our position matched our target  */
    // )   motorOff();                             // We stop trying to reach our target, lest we look like fools!
    
    return nullptr;

}