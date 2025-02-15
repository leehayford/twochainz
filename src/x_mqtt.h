#ifndef X_MQTT_H
#define X_MQTT_H

#include "dc_esp_server.h"
#include "dc_alert.h"
#include "x_models.h"
#include "x_io.h"

/* MQTT Pubclications *************************************************************************************/

#define N_PUBS 6

typedef enum  {
    PUB_ERROR = 0,
    PUB_ADMIN,
    PUB_STATE,
    PUB_CONFIG,
    PUB_OPS,
    PUB_OPS_POS
} eMqttPubMap_t;

extern void setMQTTPubFlag(eMqttPubMap_t pub);

extern void opsStatusReport(const char* status_msg);

extern void diagnosticReoprt();

extern void mqttPublishAlert(Alert* alert);

void mqttPublishAdmin();

void mqttPublishState();

void mqttPublishConfig();

void mqttPublishOps(); 

// Publishes Ops.currentHeight only (float inches)  
void mqttPublishOpsPosition();

/* MQTT Pubclications *** END *****************************************************************************/  



/* MQTT Subscriptions *************************************************************************************/

#define N_SUBS 23
/* Message IGNORED 
Sets MQTT publish flags: 
- Admin
- Config (if Config.run)
- State
- Ops */
void mqttHandleCMDReport(char* msg);

/* Message PARSED --> Admin 
Sets administrator level values
Sets MQTT publish flag:
- Admin
*/
void mqttHandleCMDAdmin(char* msg);

/* Message PARSED --> Admin 
Sets administrator level values
Saves administrator level values to /adm.js
Sets MQTT publish flag:
- Admin
*/
void mqttHandleCMDAdminSetDefaults(char* msg);

/* Message IGNORED 
Reads administrator level values from /adm.js
Sets administrator level values
Sets MQTT publish flag:
- Admin
*/
void mqttHandleCMDAdminGetDefaults(char* msg);

/* Message IGNORED 
Sets MQTT publish flag: State */
void mqttHandleCMDState(char* msg);

/* Message PARSED --> Config
Clears Ops progress
Sets Ops return to home flags
Sets MQTT publish flags: 
- Config 
- State
- Ops */
void mqttHandleCMDConfig(char* msg);

/* Message IGNORED 
Sets MQTT publish flag: Ops */
void mqttHandleCMDOps(char* msg);

/* Message IGNORED 
Clears Config
Clears Ops progress
Sets Ops return to home flags
Clears Ops.wantAid flag
Sets MQTT publish flags: 
- Config 
- State
- Ops */ 
void mqttHandleCMDOpsReset(char* msg);

/* Message IGNORED ... */
void mqttHandleCMDOpsRun(char* msg);

/* Message IGNORED ... */
void mqttHandleCMDOpsPause(char* msg);

/* Message IGNORED 
Clears Ops.wantAid flag
Previous operation resumes 
Sets MQTT publish flags: 
- Config 
- State
- Ops */ 
void mqttHandleCMDOpsContinue(char* msg);

/* Message IGNORED */
void mqttHandleCMDEnableAWESMode(char* msg); 

/* Message IGNORED */
void mqttHandleCMDDisableAWESMode(char* msg);


/* DIAGNOSTIC COMMANDS ****************************************************************/

/* Message IGNORED */
void mqttHandleCMDEnableDiagnostics(char* msg);
/* Message IGNORED */
void mqttHandleCMDDisableDiagnostics(char* msg);

/* Message IGNORED */
void mqttHandleCMDBrakeOn(char* msg);
/* Message IGNORED */
void mqttHandleCMDBrakeOff(char* msg);

/* Message IGNORED */
void mqttHandleCMDMagnetOn(char* msg);
/* Message IGNORED */
void mqttHandleCMDMagnetOff(char* msg);

/* Message IGNORED 
    - Calls diagnosticMove(true)
    - Moves UP g_admin.diagSteps @ g_admin.motHzLow */
void mqttHandleCMDMoveUp(char* msg);
/* Message IGNORED 
    - Calls diagnosticMove(false)
    - Moves DOWN g_admin.diagSteps @ g_admin.motHzLow */
void mqttHandleCMDMoveDown(char* msg);

/* Message IGNORED */
void mqttHandleCMDMotorStop(char* msg);
/* Message IGNORED */
void mqttHandleCMDMotorZero(char* msg);

/* END DIAGNOSTIC COMMANDS ************************************************************/

/* MQTT Subscriptions *** END *****************************************************************************/


extern void mqttCallBack_X(char* topic, byte* message, unsigned int length);

extern void setupMQTT_X(const char* mqttBrokerIP, int mqttBrokerPort);

extern void serviceMQTTClient_X(const char* user, const char* pw);

#endif /* X_MQTT_H */
