#include "x_mqtt.h"


/* MQTT Pubclications *************************************************************************************/
mqttPublication m_mqttPubs[N_PUBS] = {
    {(char*)"esp32/sig/state", 0, (mqttPubFunc)&mqttPublishState},
    {(char*)"esp32/sig/config", 0, (mqttPubFunc)&mqttPublishConfig},
    {(char*)"esp32/sig/error", 0, (mqttPubFunc)&mqttPublishError},
    {(char*)"esp32/sig/motpos", 0, (mqttPubFunc)&mqttPublishMotorPosition},
};

void setMQTTPubFlag(eMqttPubMap_t pub) {
    m_mqttPubs[pub].flag = 1;
}

void mqttPublishState() { 
    publishMQTTMessage(m_mqttPubs[PUB_STATE].topic, (char *)g_state.serializeToJSON()); 
}
void mqttPublishConfig() { 
    publishMQTTMessage(m_mqttPubs[PUB_CONFIG].topic, (char *)g_config.serializeToJSON()); 
}
void mqttPublishError() { 
    publishMQTTMessage(m_mqttPubs[PUB_ERROR].topic, (char *)"ERROR: PLACE HOLDER..."); 
}

void mqttPublishMotorPosition() {
    char buffer[20]; 
    snprintf(buffer, sizeof(buffer), "%.8f", g_state.currentHeight);
    publishMQTTMessage(m_mqttPubs[PUB_MOTPOS].topic, buffer);
}


/* MQTT Subscriptions *************************************************************************************/
mqttSubscription m_mqttSubs[N_SUBS] = {
    {(char*)"esp32/cmd/report", (mqttCMDFunc)&mqttHandleCMDReport},
    {(char*)"esp32/cmd/state", (mqttCMDFunc)&mqttHandleCMDState},
    {(char*)"esp32/cmd/config", (mqttCMDFunc)&mqttHandleCMDConfig},
    {(char*)"esp32/cmd/error", (mqttCMDFunc)&mqttHandleCMDError},
};

void mqttHandleCMDReport(char* msg) {
    setMQTTPubFlag(PUB_CONFIG);
    setMQTTPubFlag(PUB_STATE);
    setMQTTPubFlag(PUB_MOTPOS);
}

void mqttHandleCMDState(char* msg) { 
    setMQTTPubFlag(PUB_STATE);
}

void mqttHandleCMDConfig(char* msg) {
    g_config.parseFromJSON(msg);
    g_state.cyclesCompleted = 0;
    g_state.currentHeight = 0.0; /* TODO: CHECK / GO HOME FIRST */
    motorSetPositionAsZero();
    setMQTTPubFlag(PUB_CONFIG);
    setMQTTPubFlag(PUB_STATE);
    setMQTTPubFlag(PUB_MOTPOS);
}

void mqttHandleCMDError(char* msg) { /* TODO: CLEAR ERRORS... */

}

/* MQTT General Setup *************************************************************************************/
void mqttCallBack_X(char* topic, byte* message, unsigned int length) {
    Serial.printf("\nMessage arrived on topic: %s", topic);
    for (mqttSubscription &sub : m_mqttSubs) {
        if (strcmp(topic, sub.topic) == 0) {
            sub.func((char *)message);
            break;
        }
    }
}

void setupMQTT_X(const char* mqttBrokerIP, int mqttBrokerPort) {
    setupMQTTClient(mqttBrokerIP, mqttBrokerPort,(mqttCallBackFunc)&mqttCallBack_X);
}

void serviceMQTTClient_X(const char* user, const char* pw) {
    serviceMQTTClient(user, pw, m_mqttSubs, N_SUBS);
    for (mqttPublication &pub : m_mqttPubs) {
        if ( pub.flag > 0 ) {
            pub.func();
            pub.flag = 0;
        }
    }
}
