#ifndef X_MQTT_H
#define X_MQTT_H

#include "dc_esp_server.h"
#include "x_models.h"


#define N_PUBS 3
void mqttPublishState();
void mqttPublishConfig();
void mqttPublishError();
typedef enum  {
    PUB_STATE = 0,
    PUB_CONFIG,
    PUB_ERROR
} eMqttPubMap_t;
extern void setMQTTPubFlag(eMqttPubMap_t pub);


#define N_SUBS 2
void mqttHandleCMDState(char* msg);
void mqttHandleCMDConfig(char* msg);



extern void mqttCallBack_X(char* topic, byte* message, unsigned int length);

extern void setupMQTT_X(const char* mqttBrokerIP, int mqttBrokerPort);

extern void serviceMQTTClient_X(const char* user, const char* pw);

#endif /* X_MQTT_H */
