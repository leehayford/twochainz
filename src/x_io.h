#ifndef X_IO_H
#define X_IO_H

#include <ESP_FlexyStepper.h>
#include "x_models.h"

/* INTERRUPTS *****************************************************************************************/
#define PIN_ITR_ESTOP 4// Interrupt -> Input Pullup -> Low = Emergency stop button pressed
#define PIN_ITR_DOOR 16 // Interrupt -> Input Pullup -> Low = Door is closed
#define PIN_ITR_FIST 34 // Interrupt -> Input Pullup -> Low = Fist is in contact with hammer
#define PIN_ITR_ANVIL 35 // Interrupt -> Input Pullup -> Low = Hammer is in contact with anvil 
#define PIN_ITR_TOP 32 // Interrupt -> Input Pullup -> Low = Fist is at top 
#define PIN_ITR_PRESSURE 33 // Interrupt -> Input Pullup -> Low = Brake has pressure 

/* INTERRUPT PINS -> DEBOUNCE */
#define ITR_DEBOUNCE_ALARM_INC_mSEC 50
#define ITR_DEBOUNCE_TIMER_PERIOD_uSEC 10000
#define ITR_DEBOUNCE_TIMER 0 // Timer 0
#define ITR_DEBOUNCE_PRESCALE 80 // Prescale of 80 = 1MHz for ESP32 DevKit V1
#define ITR_DEBOUNCE_COUNT_UP true // Timer counts up as opposed to down
#define ITR_DEBOUNCE_EDGE true // No idea what this is; must be set true...
#define ITR_DEBOUNCE_AUTORUN true // set 'autoreload' true to run continuously
// #define ITR_DEBOUNCE_RUN_ONCE false // set 'autoreload' false to run once

/* INTERRUPTS *** END ********************************************************************************/



/* RELAY CONTROL *************************************************************************************/
#define PIN_OUT_BRAKE 23 // Relay Control -> Output -> Low = Brake is engaged
#define BREAK_ON LOW
#define BREAK_OFF HIGH

#define PIN_OUT_MAGNET 22 // Relay Control -> Output -> Low = Brake is engaged
#define MAGNET_ON HIGH
#define MAGNET_OFF LOW

/* RELAY CONTROL *** END ***************************************************************************/



/* MOTOR CONTROL ***********************************************************************************/
#define PIN_OUT_MOT_STEP 26 // Stepper Motor Control -> Output -> One pulse per step
#define PIN_OUT_MOT_DIR 25 // Stepper Control Control -> Output -> High = Up
#define PIN_OUT_MOT_EN 13 // Stepper Control Control -> Output -> High = Enabled

/* TODO: UNDEFINE TEST_STEP_DRIVER FOR PRODUCTION */ #define TEST_STEP_DRIVER
#ifdef TEST_STEP_DRIVER
extern int steps;
extern void motorEnable();
extern void motorBackNForth();
#endif /* TEST_STEP_DRIVER */

#define MOT_ENABLED LOW
#define MOT_DISABLED HIGH
#define MOT_DIR_UP HIGH
#define MOT_DIR_DOWN LOW
#define MOT_STEPS_PER_SEC_LOW 500
#define MOT_STEPS_PER_SEC_HIGH 4000
#define MOT_STEPS_PER_SEC_ACCEL 2000
#define MOT_SERVICE_CORE 0 // Which processor core onwhich to run the stepper service 

#define MOT_STEP_PER_REV 2000

#define FIST_INCH_PER_REV 6.000
#define FIST_HEIGHT_MAX_INCH 48.000
#define FIST_HEIGHT_MAX_STEP 16000 // = ( 48 / 6 ) * 2000

/* MOTOR CONTROL *** END *************************************************************************/



extern void setupIO();

#endif /* X_IO_H */