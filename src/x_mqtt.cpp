#include "x_mqtt.h"



/* MQTT Subscriptions *************************************************************************************/
void mqttHandleCMDState(char* msg) { 
    // sta.parseFromJSON(msg);
    pubs[PUB_STATE].flag = 1; 
}

void mqttHandleCMDConfig(char* msg) {
    cfg.parseFromJSON(msg);
    sta.cyclesCompleted = 0;
    pubs[PUB_CONFIG].flag = 1;
}

mqttSubscription subs[N_SUBS] = {
    {(char*)"esp32/cmd/state", (mqttCMDFunc)&mqttHandleCMDState},
    {(char*)"esp32/cmd/config", (mqttCMDFunc)&mqttHandleCMDConfig},
};

/* MQTT Pubclications *************************************************************************************/
void mqttPublishState() { 
    publishMQTTMessage(pubs[PUB_STATE].topic, (char *)sta.serializeToJSON()); 
}
void mqttPublishConfig() { 
    publishMQTTMessage(pubs[PUB_CONFIG].topic, (char *)cfg.serializeToJSON()); 
}
void mqttPublishError() { 
    publishMQTTMessage(pubs[PUB_ERROR].topic, (char *)"ERROR: PLACE HOLDER..."); 
}

mqttPublication pubs[N_PUBS] = {
    {(char*)"esp32/sig/state", 0, (mqttPubFunc)&mqttPublishState},
    {(char*)"esp32/sig/config", 0, (mqttPubFunc)&mqttPublishConfig},
    {(char*)"esp32/sig/error", 0, (mqttPubFunc)&mqttPublishError},
};

void serviceMQTTClient_X(const char* user, const char* pw) {
    serviceMQTTClient(user, pw, subs, N_SUBS);
    for (mqttPublication &pub : pubs) {
        if ( pub.flag > 0 ) {
            pub.func();
            pub.flag = 0;
        }
    }
}


void mqttCallBack_X(char* topic, byte* message, unsigned int length) {
    Serial.printf("Message arrived on topic: %s", topic);
    for (mqttSubscription sub : subs) {
        if (strcmp(topic, sub.topic) == 0) {
            sub.func((char *)message);
            break;
        }
    }
}

void setupMQTT_X(const char* mqttBrokerIP, int mqttBrokerPort) {
    setupMQTTClient(mqttBrokerIP, mqttBrokerPort,(mqttCallBackFunc)&mqttCallBack_X);
}