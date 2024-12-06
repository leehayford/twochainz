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


extern void statusUpdate(const char* status_msg);
extern void setupOps();
extern void runOperations();

#endif /* X_MACHINE_H */
