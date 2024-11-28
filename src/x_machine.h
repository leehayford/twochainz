#ifndef X_MACHINE_H
#define X_MACHINE_H
#include "x_models.h"
#include "x_mqtt.h"
#include "x_io.h"

/* ALARMS *********************************************************************************************/
#define STATUS_START "Move Bitch! Get out the way!"
#define STATUS_ESTOP "EMERGENCEY STOP"
#define STATUS_DOOR_OPEN "DOOR OPEN"
#define STATUS_REQUEST_HELP "SEEKING OPERATER ASSISTANCE"
#define STATUS_RECOVERY "LOST POSITION FIX"
#define STATUS_SEEK_HAMMER "SEEKING HAMMER"
#define STATUS_SEEK_ANVIL "SEEKING ANVIL"
#define STATUS_AWAIT_CONFIG "READY FOR CONFIGURATION"
#define STATUS_FIST_OPEN "FIST OPEN"
#define STATUS_ANVIL_LIMIT "ANVIL CONTACT"
#define STATUS_TOP_LIMIT "TOP LIMIT FAULT"
#define STATUS_PRESSURE "BRAKE PRESSURE FAULT"

#endif /* X_MACHINE_H */