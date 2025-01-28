#ifndef X_MACHINE_H
#define X_MACHINE_H

// #include "dc_error.h"
#include "dc_alert.h"
#include "x_models.h"
#include "x_mqtt.h"
#include "x_io.h"

/* ALARMS *********************************************************************************************/

#define STATUS_PRESSURE "BRAKE PRESSURE FAULT"
#define STATUS_HAMMERTIME_OUT "HAMMERTIME OUT FAULT"

#define OPS_ITR_TIMER 1                             // Timer 1
#define OPS_ITR_TIMER_PRESCALE 80                   // Prescale of 80 = 1MHz for ESP32 DevKit V1
#define OPS_ITR_TIMER_EDGE true                     // No idea what this is; must be set true...
#define OPS_ITR_TIMER_AUTORUN true                  // set 'autoreload' true to run continuously
#define OPS_ITR_TIMER_COUNT_UP true                 // Timer counts up as opposed to down
#define OPS_ITR_TIMER_PERIOD_uSEC 10000             // 0.01 seconds between interrupt flag checks

#define OPS_HAMMER_TIMER 2                          // Timer 2
#define OPS_HAMMER_TIMER_PRESCALE 80                // Prescale of 80 = 1MHz for ESP32 DevKit V1
#define OPS_HAMMER_TIMER_EDGE true                  // No idea what this is; must be set true...
#define OPS_HAMMER_TIMER_RUN_ONCE false             // set 'autoreload' false to run once
#define OPS_HAMMER_TIMER_COUNT_UP true              // Timer counts up as opposed to down
#define OPS_HAMMER_TIMER_STRIKE_PERIOD_uSEC 1000000 // 1.0 seconds for hammer to drop

// #define OPS_BRAKE_TIMER 3                           // Timer 3
// #define OPS_BRAKE_TIMER_PRESCALE 80                 // Prescale of 80 = 1MHz for ESP32 DevKit V1
// #define OPS_BRAKE_TIMER_EDGE true                   // No idea what this is; must be set true...
// #define OPS_BRAKE_TIMER_RUN_ONCE false             // set 'autoreload' false to run once
// #define OPS_BRAKE_TIMER_COUNT_UP true               // Timer counts up as opposed to down
// #define OPS_BRAKE_TIMER_ON_PERIOD_uSEC 300000       // 0.3 seconds for pressure to drop
// #define OPS_BRAKE_TIMER_OFF_PERIOD_uSEC 1000000     // 1.0 seconds for pressure to build

#define POS_UPDATE_PERIOD_mSec 300

/* Deal with operational alerts 
Where alert code == ERROR
    - Stop the motor
    - Apply the brake
    - Wait for operator assistance
Regardless of alert code:
    - publish alert
*/
extern void doOperationsAlert(Alert* alert);

extern void statusUpdate(const char* status_msg);
extern void setupOps();
extern Alert* runOperations();
extern void schedulePositionUpdate();
extern Alert* doPositionUpdate();

#endif /* X_MACHINE_H */
