#ifndef X_MQTT_H
#define X_MQTT_H

#include "dc_esp_server.h"
#include "x_models.h"

#define N_SUBS 2
extern mqttSubscription subs[];
extern void setupXMQTT(const char* mqttBrokerIP, int mqttBrokerPort);
extern void mqttCallBack(char* topic, byte* message, unsigned int length);

#endif /* X_MQTT_H */
