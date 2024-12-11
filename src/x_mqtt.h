#ifndef X_MQTT_H
#define X_MQTT_H

#include "dc_esp_server.h"
#include "dc_error.h"
#include "x_models.h"
#include "x_io.h"

#define MQTT_TOPIC_PREFIX "esp32/"

typedef enum  {
    PUB_ERROR = 0,
    PUB_STATE,
    PUB_CONFIG,
    PUB_OPS,
    PUB_OPS_POS
} eMqttPubMap_t;

extern void setMQTTPubFlag(eMqttPubMap_t pub);

#define N_PUBS 5

extern void mqttPublishError(Error* err);

void mqttPublishState();

void mqttPublishConfig();

void mqttPublishOps(); 

// Publishes Ops.currentHeight only (float inches)  
void mqttPublishOpsPosition();  


#define N_SUBS 8

/* Message IGNORED 
Sets MQTT publish flags: 
- Config 
- State
- Ops */
void mqttHandleCMDReport(char* msg);

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

/* Message IGNORED 
Clears Ops.wantAid flag
Previous operation resumes 
Sets MQTT publish flags: 
- Config 
- State
- Ops */ 
void mqttHandleCMDOpsContinue(char* msg);



extern void mqttCallBack_X(char* topic, byte* message, unsigned int length);

extern void setupMQTT_X(const char* mqttBrokerIP, int mqttBrokerPort);

extern void serviceMQTTClient_X(const char* user, const char* pw);

#endif /* X_MQTT_H */
