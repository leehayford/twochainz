#ifndef X_IO_H
#define X_IO_H

// #include <ESP_FlexyStepper.h>
#include "x_models.h"


/* INTERRUPT PINS *************************************************************************************/
#define PIN_ITR_ESTOP 36 // Interrupt -> Input Pullup -> Low = Emergency stop button pressed
#define PIN_ITR_DOOR 39 // Interrupt -> Input Pullup -> Low = Door is closed
#define PIN_ITR_ARM 34 // Interrupt -> Input Pullup -> Low = Arm is in contact with hammer
#define PIN_ITR_ANVIL 35 // Interrupt -> Input Pullup -> Low = Hammer is in contact with anvil 

/* INTERRUPT PINS -> DEBOUNCE */
#define ITR_DEBOUNCE_ALARM_INC_mSEC 2
#define ITR_DEBOUNCE_TIMER_PERIOD_uSEC 1000
#define ITR_DEBOUNCE_TIMER 0 // Timer 0
#define ITR_DEBOUNCE_PRESCALE 80 // Prescale of 80 = 1MHz for ESP32 DevKit V1
#define ITR_DEBOUNCE_COUNT_UP true // Timer counts up as opposed to down
#define ITR_DEBOUNCE_EDGE true // No idea what this is; must be set true...
// #define ITR_DEBOUNCE_RUN_ONCE false // set 'autoreload' false to run once
#define ITR_DEBOUNCE_AUTORUN true // set 'autoreload' true to run continuously

/* RELAY CONTROL PINS *******************************************************************************/
#define PIN_OUT_NH_BRAKE 32 // Relay Control -> Output NH -> Low = Brake is engaged
#define PIN_OUT_NH_MAGNET 33 // Relay Control -> Output NH -> Low = Brake is engaged

/* MOTOR CONTROL PINS *****************************************************************************/
#define PIN_OUT_MOT_STEP 25 // Stepper Motor Control -> Output -> One pulse per step
#define PIN_OUT_MOT_DIR 26 // Stepper Control Control -> Output -> High = Up
#define PIN_OUT_MOT_EN 27 // Stepper Control Control -> Output -> High = Enabled

/* MOTOR CONTROL -> MICRO STEP CONTROL PINS
    MS1     MS2     MS3
        0           0           0       ->      Full step 1.8°
        1           0           0       ->      Half step 0.9°
        0           1           0       ->      Quarter step 0.45°
        1           1           0       ->      Eighth step 0.225°
        1           1           1       ->      Sixteenth step 0.1125°
*/
#define PIN_OUT_MOT_MS1 27 // Stepper Control Control -> Output NH 
#define PIN_OUT_MOT_MS2 14 // Stepper Control Control -> Output NH 
#define PIN_OUT_MOT_MS3 12 // Stepper Control Control -> Output NH 

#define MOT_ENABLED HIGH
#define MOT_DISABLED LOW
#define MOT_DIR_UP HIGH
#define MOT_DIR_DOWN LOW
#define MOT_STEPS_PER_SEC 2000
#define MOT_ACCEL_STEPS_PER_SEC 5000
#define MOT_SERVICE_CORE 0 // Which processor core onwhich to run the stepper service 

extern void setupIO();

#endif /* X_IO_H */