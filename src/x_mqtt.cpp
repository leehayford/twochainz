#include "x_mqtt.h"


/* MQTT Pubclications *************************************************************************************/
char* SIG = mqttTopic(MQTT_TOPIC_PREFIX, "sig/");

void setMQTTPubFlag(eMqttPubMap_t pub) {
    m_mqttPubs[pub].flag = 1;
}

mqttPublication m_mqttPubs[N_PUBS] = {
    {mqttTopic(SIG, "error"), 0, (mqttPubFunc)&mqttPublishError},
    {mqttTopic(SIG, "state"), 0, (mqttPubFunc)&mqttPublishState},
    {mqttTopic(SIG, "config"), 0, (mqttPubFunc)&mqttPublishConfig},

    {mqttTopic(SIG, "ops"), 0, (mqttPubFunc)&mqttPublishOps},
    {mqttTopic(SIG, "ops/pos"), 0, (mqttPubFunc)&mqttPublishOpsPosition},
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


/* MQTT Subscriptions *************************************************************************************/
char* CMD = mqttTopic(MQTT_TOPIC_PREFIX, "cmd/");

mqttSubscription m_mqttSubs[N_SUBS] = {
    {mqttTopic(CMD, "report"), (mqttCMDFunc)&mqttHandleCMDReport},
    {mqttTopic(CMD, "state"), (mqttCMDFunc)&mqttHandleCMDState},
    {mqttTopic(CMD, "config"), (mqttCMDFunc)&mqttHandleCMDConfig},

    {mqttTopic(CMD, "ops"), (mqttCMDFunc)&mqttHandleCMDOps},
    {mqttTopic(CMD, "ops/reset"), (mqttCMDFunc)&mqttHandleCMDOpsReset},
    {mqttTopic(CMD, "ops/continue"), (mqttCMDFunc)&mqttHandleCMDOpsContinue},
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
    g_ops.clearProgress();
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
