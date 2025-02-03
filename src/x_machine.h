#ifndef X_MACHINE_H
#define X_MACHINE_H

#include "dc_esp_server.h"
#include "dc_alert.h"
#include "x_models.h"
#include "x_mqtt.h"
#include "x_io.h"

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

extern Alert* runOperations();

extern void schedulePositionUpdate();

extern Alert* doPositionUpdate();

#endif /* X_MACHINE_H */
