#ifndef X_MACHINE_H
#define X_MACHINE_H

#include "dc_error.h"
#include "x_models.h"
#include "x_mqtt.h"
#include "x_io.h"

/* ALARMS *********************************************************************************************/
#define STATUS_START "IICHAINZ INITIALIZED"

#define STATUS_ESTOP "EMERGENCEY STOP"
#define STATUS_DOOR_OPEN "DOOR OPEN"

#define STATUS_REQUEST_HELP "SEEKING OPERATER ASSISTANCE"
#define STATUS_RECOVERY "LOST POSITION FIX"

#define STATUS_GO_HOME "GO HOME"
#define STATUS_SEEK_HAMMER "SEEKING HAMMER"
#define STATUS_SEEK_ANVIL "SEEKING ANVIL"
#define STATUS_SEEK_HOME "SEEKING HOME"
#define STATUS_AWAIT_CONFIG "READY FOR CONFIGURATION"
#define STATUS_RAISE_HAMMER "RAISING HAMMER"
#define STATUS_DROP_HAMMER "MOVE B@%?#! GET OUT THE WAY!"

#define STATUS_TOP_LIMIT "TOP LIMIT FAULT"
#define STATUS_PRESSURE "BRAKE PRESSURE FAULT"
#define STATUS_HAMMERTIME_OUT "HAMMER-TIME FAULT"


#define OPS_ITR_TIMER 1                         // Timer 1
#define OPS_ITR_TIMER_PRESCALE 80               // Prescale of 80 = 1MHz for ESP32 DevKit V1
#define OPS_ITR_TIMER_PERIOD_uSEC 2000
#define OPS_ITR_TIMER_EDGE true                 // No idea what this is; must be set true...
#define OPS_ITR_TIMER_AUTORUN true              // set 'autoreload' true to run continuously
#define OPS_ITR_TIMER_COUNT_UP true             // Timer counts up as opposed to down

#define HAMMER_STRIKE_TIMER 2                   // Timer 2
#define HAMMER_STRIKE_TIMER_PRESCALE 80         // Prescale of 80 = 1MHz for ESP32 DevKit V1
#define HAMMER_STRIKE_TIMER_PERIOD_uSEC 500
#define HAMMER_STRIKE_TIMER_EDGE true           // No idea what this is; must be set true...
#define HAMMER_STRIKE_TIMER_RUN_ONCE false      // set 'autoreload' false to run once
#define HAMMER_STRIKE_TIMER_COUNT_UP true       // Timer counts up as opposed to down

#define POS_UPDATE_PERIOD_mSec 300

extern void statusUpdate(const char* status_msg);
extern void setupOps();
extern Error* runOperations();

#endif /* X_MACHINE_H */
