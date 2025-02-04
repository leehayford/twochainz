#ifndef X_MACHINE_H
#define X_MACHINE_H

#include "dc_esp_server.h"
#include "dc_alert.h"
#include "x_models.h"
#include "x_mqtt.h"
#include "x_io.h"

extern void statusUpdate(const char* status_msg);

extern void checkDiagnostics();

extern bool isOperatingFaultCondition();

extern Alert* runOperations();

extern void schedulePositionUpdate();

extern void doPositionUpdate();

#endif /* X_MACHINE_H */
