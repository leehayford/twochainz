#ifndef X_MACHINE_H
#define X_MACHINE_H

#include "dc_error.h"
#include "x_models.h"
#include "x_mqtt.h"
#include "x_io.h"

/* ALARMS *********************************************************************************************/



#define STATUS_TOP_LIMIT "TOP LIMIT FAULT"
#define STATUS_PRESSURE "BRAKE PRESSURE FAULT"
#define STATUS_HAMMERTIME_OUT "HAMMERTIME OUT FAULT"

#define OPS_ITR_TIMER 1                             // Timer 1
#define OPS_ITR_TIMER_PRESCALE 80                   // Prescale of 80 = 1MHz for ESP32 DevKit V1
#define OPS_ITR_TIMER_EDGE true                     // No idea what this is; must be set true...
#define OPS_ITR_TIMER_AUTORUN true                  // set 'autoreload' true to run continuously
#define OPS_ITR_TIMER_COUNT_UP true                 // Timer counts up as opposed to down
#define OPS_ITR_TIMER_PERIOD_uSEC 10000             // 0.01 seconds between interrupt flag checks

#define OPS_BRAKE_TIMER 2                           // Timer 2
#define OPS_BRAKE_TIMER_PRESCALE 80                 // Prescale of 80 = 1MHz for ESP32 DevKit V1
#define OPS_BRAKE_TIMER_EDGE true                   // No idea what this is; must be set true...
#define OPS_BRAKE_TIMER_RUN_ONCE false             // set 'autoreload' false to run once
#define OPS_BRAKE_TIMER_COUNT_UP true               // Timer counts up as opposed to down
#define OPS_BRAKE_TIMER_ON_PERIOD_uSEC 300000       // 0.3 seconds for pressure to drop
#define OPS_BRAKE_TIMER_OFF_PERIOD_uSEC 1000000     // 1.0 seconds for pressure to build

#define OPS_HAMMER_TIMER 3                          // Timer 3
#define OPS_HAMMER_TIMER_PRESCALE 80                // Prescale of 80 = 1MHz for ESP32 DevKit V1
#define OPS_HAMMER_TIMER_EDGE true                  // No idea what this is; must be set true...
#define OPS_HAMMER_TIMER_RUN_ONCE false             // set 'autoreload' false to run once
#define OPS_HAMMER_TIMER_COUNT_UP true              // Timer counts up as opposed to down
#define OPS_HAMMER_TIMER_CATCH_PERIOD_uSEC 200000   // 0.2 seconds for delay between dropping and catching the hammer
// #define OPS_HAMMER_TIMER_STRIKE_PERIOD_uSEC 1000000 // 1.0 seconds for hammer to drop

#define HAMMER_STRIKE_mSEC 1000                     // 1.0 seconds for hammer to drop

#define POS_UPDATE_PERIOD_mSec 300

extern void statusUpdate(const char* status_msg);
extern void setupOps();
extern Error* runOperations();

#endif /* X_MACHINE_H */
