#ifndef X_MQTT_H
#define X_MQTT_H

#include "dc_esp_server.h"
#include "x_models.h"

#define N_SUBS 2
extern mqttSubscription subs[];
extern void mqttCallBack_X(char* topic, byte* message, unsigned int length);

#define N_PUBS 3
extern mqttPublication pubs[];
enum mqttPubMap {
    PUB_STATE = 0,
    PUB_CONFIG,
    PUB_ERROR
};
extern void serviceMQTTClient_X(const char* user, const char* pw);

extern void setupMQTT_X(const char* mqttBrokerIP, int mqttBrokerPort);

#endif /* X_MQTT_H */
