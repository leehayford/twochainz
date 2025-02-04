#include "x_machine.h"

void doOperationsAlert(Alert* alert) {
    
    if( alert->getCode() == ERROR) {        /* We have an ERROR alert */
        motorStop();                        // We stop going places, to avoid making things worse
        brakeOn();                          // We apply the brake
        g_ops.wantAid = true;               // We take ownership of our need for help
    }
    mqttPublishAlert(alert);                // We tell all of our troubles
}

void statusUpdate(const char* status_msg) {
    // Serial.printf("\nstatusUpdate(%s)", status_msg);
    g_ops.setStatus(status_msg);
    setMQTTPubFlag(PUB_CONFIG);
    setMQTTPubFlag(PUB_STATE);
    setMQTTPubFlag(PUB_OPS);
}

void loadRecoveryCourseAndSeed() {
    g_ops.stepTarget = g_admin.getRecoverySteps();  // We will attempt to do the full 48.0" (DOWN) and reassess
    g_ops.stepHz = g_admin.motHzLow;                // We will proceed at SLOW rip 
}

void loadHomeCourseAndSeed() {
    g_ops.stepTarget = 
        (g_state.motorSteps + g_admin.motStepsOver) 
        * -1;                                       // We set our course for home (DOWN)

    g_ops.stepHz = g_admin.motHzHigh;               // We will proceed at FULL RIP!
}

void loadQuestCourseAndSpeed() {
    // We set our course for the height of configuration (UP)
    g_ops.stepTarget = (g_config.height / g_admin.motInchRev) * g_admin.motStepsRev;

    g_ops.stepHz = g_admin.motHzHigh;           // We will proceed at FULL RIP!  
    
    // Serial.printf("\nx_machine getQuestCourseAndSpeed:\tSTEPS: %d\tHz: %d\n", g_ops.stepTarget, g_ops.stepHz); 
}
 
void moveToTarget() {
   
    if( g_ops.reorient                          /* We are lost and being guided home */
    )   loadRecoveryCourseAndSeed();            // We will move as we are guided

    else {                                      /* We must assume we're on track */

        motorGetPosition();                     // We take a bearing
                                          
        if( g_ops.goHome                        /* We've been ordered home */
        )   loadHomeCourseAndSeed();            // We will make for home                     
        
        else                                            
        if( g_ops.raiseHammer                   /* We've been ordered to raise the hammer */
        )   loadQuestCourseAndSpeed();          // We will make for the heigh of configuration
    }     

    motorSetSpeed(g_ops.stepHz);
    motorSetCourse(g_ops.stepTarget);           // We get to steppin'
    // Serial.printf(
    //     "\nx_machine moveToTarget:\tSTEPS: %d\tHz: %d\n", 
    //     g_ops.stepTarget, 
    //     g_ops.stepHz
    // ); 
   
}

/* FAULT CHECKS ***************************************************************************************/



Alert ALERT_ESTOP("emergency stop has been initiated", ERROR);
/* Called by isOperatingFaultCondition()
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
        motorStop();                            // We stop going places, to avoid making things worse
        brakeOn();                              // We apply the brake
        g_ops.wantAid = true;                   // We take ownership of our need for help
        
        if( !g_ops.wantEStopRelease             /* We were unaware of this pressedness */
        ) {
            mqttPublishAlert(&ALERT_ESTOP);     // We expose this treachery!
            statusUpdate(STATUS_ESTOP);         // We expose it ALL!
            g_ops.wantEStopRelease = true;      // We yearn for release
        }
    }
       
    else                                          
    if( !g_state.eStop                          /* Release has been given */ 
    &&  g_ops.wantEStopRelease                  /* Still we yearn for release */
    ) {  
        g_ops.wantEStopRelease = false;         // We stop yearning for release, lest we look like perverts!
        statusUpdate((char*)"...");
    }
    return g_ops.wantEStopRelease; 
}

Alert ALERT_DOOR_OPEN("door open; 2chainz has been violated", ERROR);
/* Called by isOperatingFaultCondition()
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
        motorStop();                            // We stop going places, to avoid making things worse
        brakeOn();                              // We apply the brake
        g_ops.wantAid = true;                   // We take ownership of our need for help

        if( !g_ops.wantDoorClose                /* We were unaware of this violation */
        ) {
            mqttPublishAlert(&ALERT_DOOR_OPEN); // We expose this treachery!
            statusUpdate(STATUS_DOOR_OPEN);     // We expose it ALL!
            g_ops.wantDoorClose = true;         // We yearn for closure
        }
    }
     

    else                                          
    if( !g_state.doorOpen                       /* The door is closed */
    &&  g_ops.wantDoorClose                     /* Still we yearn for closure */
    ) {  
        g_ops.wantDoorClose = false;            // We stop yearning for closure, lest we appear incapable of emotional growth!
        statusUpdate((char*)"...");
    }
    
    return g_ops.wantDoorClose;
}

Alert ALERT_TOP_LIMIT("top limit fault; 2chainz is way too high right now", ERROR);
/* Called by isOperatingFaultCondition()
Returns true while the fist is too high */
bool isTopLimitFault() {
    
    if( g_state.topLimit                        /* We are way too high right now... Did somebody slip us something? */
    ) {
        motorStop();                            // We stop going places, to avoid making things worse
        brakeOn();                              // We apply the brake
        g_ops.wantAid = true;                   // We take ownership of our need for help

        if( !g_ops.wantFistDown                 /* We were unaware of our highness */
        ) {
            mqttPublishAlert(&ALERT_TOP_LIMIT); // We expose this treachery!
            statusUpdate(STATUS_TOP_LIMIT);     // We expose it ALL!
            g_ops.wantFistDown = true;          // We yearn to be less high
        }
    }
    
    else                                          
    if( !g_state.topLimit                       /* We are like 'regular high' now */
    &&  g_ops.wantFistDown                      /* Still we yearn to be less high */
    ) { 
        g_ops.wantFistDown = false;             // We stop yearning for sobriety, lest we appear light-weiths!
        statusUpdate((char*)"...");
    }
    return g_ops.wantFistDown;
}

Alert ALERT_PRESSURE_HIGH("failed to apply brake; high pressure", ERROR);
Alert ALERT_PRESSURE_LOW("failed to release brake; low pressure", ERROR);
/* Called by isOperatingFaultCondition() 
Returns true after the brake pressure timeout, 
when: 
    - the brake is ON, but pressure is LOW
    - the brake is OFF, but ressure is HIGH */
bool isPressureFault() {
    
    bool pressureFault = false;

    if( g_state.brakeTimeout                            /* We must have started the brake timer, and we must find out why... */
    ) {

        if( g_state.brakeOn                             /* We want to apply the brake */                    
        &&  g_state.pressure                            /* We still have pressure, though we want none! */
        ) {
            motorStop();                                // We stop going places, to avoid making things worse
            g_ops.wantAid = true;                       // We take ownership of our need for help
            pressureFault = true;
            
            if( !g_ops.wantBrakeOn                      /* We were unaware of our need to apply the brake */
            ) {
                mqttPublishAlert(&ALERT_PRESSURE_HIGH); // We expose this treachery!
                statusUpdate(STATUS_PRESSURE_HIGH);     // We expose it ALL!
                g_ops.wantBrakeOn = true;               // We yearn to be less high
            }
        }

        else
        if( !g_state.brakeOn                            /* We want to release the brake */
        &&  !g_state.pressure                           /* We have no pressure, though we want it all! */
        ) {
            motorStop();                                // We stop going places, to avoid making things worse
            g_ops.wantAid = true;                       // We take ownership of our need for help
            pressureFault = true;
            
            if( !g_ops.wantBrakeOff                     /* We were unaware of our need to release the brake */
            ) {
                mqttPublishAlert(&ALERT_PRESSURE_LOW);  // We expose this treachery!
                statusUpdate(STATUS_PRESSURE_LOW);      // We expose it ALL!
                g_ops.wantBrakeOff = true;              // We yearn to be less high
            }
        }

        else                                          
        if( g_state.brakeOn                             /* We tried to apply the brake */
        &&  !g_state.pressure                           /* We succeeded */
        ) { 
            g_ops.wantBrakeOn = false;                  // We stop yearning to be held, lest we appear soft!
            g_state.brakeTimeout = false;
            pressureFault = false;
            statusUpdate((char*)"...");
        }
        
        else                                          
        if( !g_state.brakeOn                            /* We tried to release the brake */
        &&  g_state.pressure                            /* We succeeded */
        ) { 
            g_ops.wantBrakeOff = false;                 // We stop yearning for sobriety, lest we appear soft!
            g_state.brakeTimeout = false;
            pressureFault = false;
            statusUpdate((char*)"...");
        }
    }
    

    return pressureFault;
}

/* Called by isOperatingFaultCondition()
Returns true until an operator hits reset or cancel:
MQTT message received at:
- .../cmd/ops/reset 
- .../cmd/ops/continue */
bool isAidRequired() {

    if( g_ops.wantAid                           /* Something bad happened, thus we yearn for aid */
    &&  !g_ops.requestAid                       /* We have yet to call for aid */
    ) {
        statusUpdate(STATUS_REQUEST_HELP);      // We swallow our pride; no Chainz is an island; 2Chainz, doubly so...
        g_ops.requestAid = true;                // We will not ask again; it was hard enough the first time...
    }

    return g_ops.wantAid;
}

/* Called by isOperatingFaultCondition() 
Returns true until a valid configuration is receved */
bool isConfigRequired() {

    if( !g_config.validate()                    /* We lack clear attainable goals */
    ) {
        if( !g_ops.wantConfig                   /* We were unaware of this personal failing */
        )  statusUpdate(STATUS_WANT_CONFIG);    // We fall to our knees, arms wide, fists clenched, and shout to he heavens, "WHAT DO YOU WANT FROM US!?"

        g_ops.wantConfig = true;                // We yearn for purpose
    }

    else                                        /* We are charged with glorious purpose */
    if( g_ops.wantConfig                        /* Still we yearn for purpose */
    ) {
        g_ops.wantConfig = false;               // We stop yearning for purpose, lest we appear ungrateful!
        g_ops.clearProgress();                  // To hell with all that has come before! Our singular concern is this moment, this quest. Onward! To glory! Or to ruin!  
        statusUpdate((char*)"...");
    }

    return g_ops.wantConfig;
}

/* Called with every execution of runOperations() */
bool isOperatingFaultCondition() {

    if( isEStopPressed()                        /* We yearn for the system to be enabled */
    ||  isDoorOpen()                            /* We yearn for the door to close */
    ||  isTopLimitFault()                       /* We yearn to be less high... of fist */
    ||  isPressureFault()                       /* We yearn for appropriate brake pressure */
    ||  isAidRequired()                         /* We yearn for assistance */
    ||  isConfigRequired()                      /* We yearn for purpose */
    )   return true;                            // We have a fault condition 

    return false;                               // We are free of fault conditions
}

void checkDiagnostics() {

    if( g_state.eStop
    ||  g_state.topLimit
    ||  g_state.homeLimit 
    ||  isPressureFault()                      
    )   motorStop();
    
    else
    if( !motorTargetReached() 
    )   doPositionUpdate();
}

/* FAULT CHECKS *** END *******************************************************************************/


/* OPERATIONS *****************************************************************************************/

int32_t nextPosUpdate = 0;
void schedulePositionUpdate() {
    nextPosUpdate = millis() + g_admin.opsPosPeriod_mSec; 
}
/* Called by runOperations() when: 
- position != taget

If position update period has passed:
- sends current position 
- schedules the next update */
void doPositionUpdate() {  

    if( millis() > nextPosUpdate                /* The time has come to publish our position */    
    ||  motorTargetReached()                    /* We want to tell everyone we have reached our target */
    ) {
        motorGetPosition();                     // Check our position
        setMQTTPubFlag(PUB_OPS_POS);            // Sing it
        schedulePositionUpdate();               // Schedule the next update 
    }
}


Alert ALERT_HAMMER_LOST("failed to find the hammer", ERROR);
/* Called by doGoHome()
Sets g_ops.seekHammer = false once we secure the hammer 
Returns an error if we fail to secure the hammer */
Alert* doSeekHammer() { 

    if( g_state.fistLimit                       /* We found the hammer */
    ) { 
        // Our search has ended.
        motorStop();                            // Stop moving while we secure the hammer 
        magnetOn();                             // Secure the hammer
        g_ops.seekHammer = false;               // Stop seeking the hammer, lest you look like a fool!
        return nullptr;                         // Take the hammer and leave this place! 
    }
    
    if( !g_ops.seekHammer                       /* We have yet to seek the hammer */
    ) {
        // Our search begins!
        g_ops.seekHammer = true;                // We seek the hammer
        magnetOn();                             // With fist ready
        brakeOn();                              // With brake on
        statusUpdate(STATUS_SEEK_HAMMER);       // Sing it

        moveToTarget();                         // We get to steppin'
        return nullptr;
    } 

    if( motorTargetReached()
    &&  !g_ops.wantAid
    ) {
        g_ops.wantAid = true;
        return &ALERT_HAMMER_LOST;
    }  

    // Our search continues...
    return nullptr;
}

Alert ALERT_ANVIL_LOST("failed to find the anvil", ERROR);
/* Called by doGoHome()
Sets g_ops.seekAnvil = false once we find the anvil 
Returns an error if we fail to find the anvil */
Alert* doSeekAnvil() {

    if( g_state.anvilLimit                      /* We found the anvil */
    ) { 
        // Our search has ended.
        motorStop();                            // Stop moving until we know what to do with ourselves
        g_ops.seekAnvil = false;                // Stop seeking the anvil, lest you look like a fool!
        return nullptr;                         // Take the anvil and leave this place... no wait that's heavy; I'll go; the anvil is yours now... I... don't really know what I'm... I just ummm, don't have anywh- Hey! where are you going? No. Sorry. You've got you own thing. You know what, I'm sure I'll figure it out; don't worry about it...
    }

    if( !g_ops.seekAnvil                        /* We have yet to seek the anvil */
    ) { 
        // Our search begins!
        g_ops.seekAnvil = true;                 // We seek the anvil
        magnetOn();                             // With fist closed
        brakeOff();                             // With brake off
        statusUpdate(STATUS_SEEK_ANVIL);        // Sing it

        moveToTarget();                         // We get to steppin'
        return nullptr;
    }

    if( motorTargetReached()
    &&  !g_ops.wantAid
    ) {
        g_ops.wantAid = true;
        return &ALERT_ANVIL_LOST;
    }

    // Our search continues...
    return nullptr;
}

Alert ALERT_HOME_LOST("failed to find home", ERROR);
/* Called by doGoHome()  
Sets g_ops.seekHome = false once we find home 
Returns an error if we fail to find home*/
Alert* doSeekHome() {
        
    if( g_state.homeLimit                       /* We found home */
    ) {     
        // Our search has ended.
        motorStop();                            // Stop moving until we know what to do with ourselves
        g_ops.seekHome = false;                 // Stop seeking home, lest you look like a fool!
        return nullptr;                         // Victory is ours!
    }

    if( !g_ops.seekHome                         /* We have yet to seek home */
    ) {  
        // Our search begins!
        g_ops.seekHome = true;                  // We seek home
        magnetOn();                             // With fist closed
        brakeOff();                             // With brake off
        motorSetSpeed(g_admin.motHzLow);        // Sloyw daaaahyrn
        statusUpdate(STATUS_SEEK_HOME);         // Sing it

        moveToTarget();                         // We get to steppin'
        return nullptr;
    }

    if( motorTargetReached()
    &&  !g_ops.wantAid
    ) {
        g_ops.wantAid = true;
        return &ALERT_HOME_LOST;
    }

    // Our search continues...
    return nullptr;
}

Alert ALERT_OPS_COMPLET("2chainz claims victory", SUCCES);
/* Called by runOperations() when: 
- the previous target was reached
- and g_ops.goHome is set 

Secures the hammer and places it on the anvil
Clears g_ops.goHome
Clears g_ops.reorient 
Sets g_ops.raiseHammer (if we have more cycles to complete)
Returns an alert 
    - upon completion of the required cycles 
    - where positional error exceeds the max */
Alert* doGoHome() {
    
    if( g_state.fistLimit
    &&  g_state.anvilLimit
    &&  g_state.homeLimit
    ) {
        motorStop();                                // Stop moving

        /* TODO: CHECK STEP ERROR */
        
        motorSetPositionAsZero();                   // Reset our home position
        g_ops.seekHammer = false;
        g_ops.seekAnvil = false;
        g_ops.goHome = false;                       // We stop going home, lest we look like fools!
        g_ops.reorient = false;                     // Clear the reorient flag (in case that's why we sought home)
        
        if( g_config.cycles > 0                     /* We have been questing */
        &&  g_ops.cycleCount < g_config.cycles      /* We have yet to finish questing */
        ) {
            // Our quest continues...
            g_ops.raiseHammer = true;               // We must raise the hammer 
            return nullptr;
        }  
        
        else {
            // Our quest has ended.
            g_config.run = false;                   // We stop seeking either glory or ruin, lest we look like fools!
            g_ops.stepTarget = 0;                   // We have taken every step
            brakeOn();                              // We feel a release of pressure
            magnetOff();                            // We let the hammer rest
            statusUpdate((char*)"DONE");
            return &ALERT_OPS_COMPLET;              // Flawless victory!
        }                         
    } 

    doPositionUpdate();                             // All yearn for word of our travels

    if( !g_state.fistLimit                          /* We yearn for the hammer */
    )   return doSeekHammer();                              

    // else                                         /* With the hammer secure in our fist */
    if( !g_state.anvilLimit                         /* We yearn for the anvil */
    )   return doSeekAnvil();                              

    // else                                         /* With the hammer secure and the anvil found */
    if( !g_state.homeLimit                          /* We yearn for home */
    )   return doSeekHome();                                


    return nullptr;
}

/* Called by runOperations() when:
- g_ops.raiseHammer is set 

Raises the hammer to 'The Height of Configuration'
Clears g_ops.raiseHammer
Sets g_ops.dropHammer */
Alert* doRaiseHammer() {

    if( g_state.motorSteps == 0                 /* We languish at home as glory beckons */             
    ) {    
        statusUpdate(STATUS_RAISE_HAMMER);      // We raise the hammer 
        magnetOn();                             // With fist closed
        brakeOff();                             // With brake off

        moveToTarget();                         // And we get to steppin'
    }
   
    doPositionUpdate();                         // All yearn for word of our travels
    
    if( motorTargetReached()                    /* We have raised the hammer */
    ) {
        g_ops.raiseHammer = false;              // We stop raising the hammer, lest we look like fools!
        g_ops.dropHammer = true;                // We now double-dog-dare ourselves to drop the hammer 
    }
    return nullptr;
}

Alert ALERT_HAMMERTIME_OUT("the hammer did not strike the anvil");
/* Called by runOperations() when:
- the previous target was reached
- and g_ops.dropHammer is set 

Clears g_ops.dropHammer 
Clears hammertimeOut flag 
Enables tmrHammerStrike 
Sets g_ops.wantStrike 
Returns an error if the time runs out */
Alert* doDropHammer () {

    if( !g_ops.wantStrike                       /* We have yet to drop the hammer */
    ) { 
        statusUpdate(STATUS_DROP_HAMMER);       // We drop the hammer 
        brakeOff();                             // With brake off
        magnetOff();                            // With fist open
        startHammerTimer();                   // We will yearn only for so long because, self-respect
        g_ops.wantStrike = true;                // Until then, we yearn for the hammer strike
    }

    if( !g_state.hammerTimeout
    )   return nullptr;                         // Go no further until m_bHammerTimeout has passed
   
    /* The m_bHammerTimeout has passed! */
    g_ops.dropHammer = false;                   // We have taken every step in our attempt to drop the hammer
    g_ops.wantStrike = false;                   // We stop yearning for the hammer strike, lest we look like fools!
  
    if( !g_state.anvilLimit                     /* The hammer has failed to strike */
    )   return &ALERT_HAMMERTIME_OUT;           // We feel shame and confusion
         
    g_ops.cycleCount++;                         // Count it!
    g_ops.goHome = true;                        // We must go home

    return nullptr;                             // Flawless victory!
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
Alert* runOperations() {

    if( isOperatingFaultCondition()             /* We have a fault condition */
    )   return nullptr;                         // We go no further... 

    // We have work to do and the freedome to do it

    if( g_ops.goHome                            /* We've been called home */
    )   return doGoHome();                      // All yearn for our return

    if( g_ops.raiseHammer                       /* We must raise the hammer */
    )   return doRaiseHammer();                 // We rasie the hammer

    if( g_ops.dropHammer                        /* We must drop the hammer */
    )   return doDropHammer();                  // We drop the hammer strike 
    
    return nullptr;

}