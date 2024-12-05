#include "x_mqtt.h"


/* MQTT Pubclications *************************************************************************************/
// char* SIG = mqttTopic(MQTT_TOPIC_PREFIX, "sig/");

mqttPublication m_mqttPubs[N_PUBS] = {
    {(char*)"esp32/sig/error", 0, (mqttPubFunc)&mqttPublishError},
    {(char*)"esp32/sig/state", 0, (mqttPubFunc)&mqttPublishState},
    {(char*)"esp32/sig/config", 0, (mqttPubFunc)&mqttPublishConfig},

    {(char*)"esp32/sig/ops", 0, (mqttPubFunc)&mqttPublishOps},
    {(char*)"esp32/sig/ops/pos", 0, (mqttPubFunc)&mqttPublishOpsPosition},
};

void mqttPublishError(char* msg) { /* TODO: CREATE ERROR CLASS & INSTANCES */
    publishMQTTMessage(m_mqttPubs[PUB_ERROR].topic, msg); 
}

void mqttPublishState() { 
    publishMQTTMessage(m_mqttPubs[PUB_STATE].topic, (char *)g_state.serializeToJSON()); 
}

void mqttPublishConfig() { 
    publishMQTTMessage(m_mqttPubs[PUB_CONFIG].topic, (char *)g_config.serializeToJSON()); 
}


void mqttPublishOps() { 
    publishMQTTMessage(m_mqttPubs[PUB_OPS].topic, (char *)g_ops.serializeToJSON()); 
}

void mqttPublishOpsPosition() {
    char buffer[20]; 
    snprintf(buffer, sizeof(buffer), "%.8f", g_state.currentHeight);
    publishMQTTMessage(m_mqttPubs[PUB_OPS_POS].topic, buffer);
}


void setMQTTPubFlag(eMqttPubMap_t pub) {
    m_mqttPubs[pub].flag = 1;
}


/* MQTT Subscriptions *************************************************************************************/
// char* CMD = mqttTopic(MQTT_TOPIC_PREFIX, "cmd/");


void mqttHandleCMDTestMotOn(char* msg) {
    motorOn();
}

void mqttHandleCMDTestMotOff(char* msg) {
    motorOff();
}

mqttSubscription m_mqttSubs[N_SUBS] = {
    {(char*)"esp32/cmd/report", (mqttCMDFunc)&mqttHandleCMDReport},
    {(char*)"esp32/cmd/state", (mqttCMDFunc)&mqttHandleCMDState},
    {(char*)"esp32/cmd/config", (mqttCMDFunc)&mqttHandleCMDConfig},

    {(char*)"esp32/cmd/ops", (mqttCMDFunc)&mqttHandleCMDOps},
    {(char*)"esp32/cmd/ops/reset", (mqttCMDFunc)&mqttHandleCMDOpsReset},
    {(char*)"esp32/cmd/ops/continue", (mqttCMDFunc)&mqttHandleCMDOpsContinue},

    {(char*)"esp32/cmd/test/mot_on", (mqttCMDFunc)&mqttHandleCMDTestMotOn},
    {(char*)"esp32/cmd/test/mot_off", (mqttCMDFunc)&mqttHandleCMDTestMotOff},
};


void mqttHandleCMDReport(char* msg) {
    setMQTTPubFlag(PUB_CONFIG);
    setMQTTPubFlag(PUB_STATE);
    setMQTTPubFlag(PUB_OPS);
}

void mqttHandleCMDState(char* msg) { 
    setMQTTPubFlag(PUB_STATE);
}

void mqttHandleCMDConfig(char* msg) {
    g_config.parseFromJSON(msg);
    mqttHandleCMDReport(msg);
}


void mqttHandleCMDOps(char* msg) {
    setMQTTPubFlag(PUB_OPS);
}

void mqttHandleCMDOpsReset(char* msg) {
    g_config.cmdReset();
    g_ops.cmdReset();
    mqttHandleCMDReport(msg);
}

void mqttHandleCMDOpsContinue(char* msg) {
    g_ops.cmdContinue();
    mqttHandleCMDReport(msg);
}

/* MQTT General Setup *************************************************************************************/
void mqttCallBack_X(char* topic, byte* message, unsigned int length) {
    Serial.printf("\nMessage arrived on topic: %s", topic);
    for (mqttSubscription sub : m_mqttSubs) {
        
        if (strcmp(topic, sub.topic) == 0) {
            Serial.printf("\nSUB: %s\n", sub.topic);
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
            // Serial.printf("\nPUB: %s\n", pub.topic);
            pub.func();
            pub.flag = 0;
        }
    }
}
