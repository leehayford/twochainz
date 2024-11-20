#include "x_mqtt.h"

void mqttHandleCMDState(char* msg) {
    publishMQTTMessage((char*)"esp32/sig/state", (char *)sta.serializeToJSON());
}

void mqttHandleCMDConfig(char* msg) {
    cfg.parseFromJSON(msg);
    sta.cyclesCompleted = 0;
    publishMQTTMessage((char*)"esp32/sig/config", (char *)cfg.serializeToJSON());
}

mqttSubscription subs[N_SUBS] = {
    {(char*)"esp32/cmd/state", (mqttCMDFunc)&mqttHandleCMDState},
    {(char*)"esp32/cmd/config", (mqttCMDFunc)&mqttHandleCMDConfig},
};

void mqttCallBack(char* topic, byte* message, unsigned int length) {

    String msgTopic = String(topic);
    Serial.println("Message arrived on topic: " + msgTopic);

    for (mqttSubscription sub : subs) {
        // if (msgTopic == sub.topic) {
        if (strcmp(topic, sub.topic) == 0) {
            sub.func((char *)message);
            break;
        }
    }

}

void setupXMQTT(const char* mqttBrokerIP, int mqttBrokerPort) {
    setupMQTTClient(mqttBrokerIP, mqttBrokerPort,(mqttCallBackFunc)&mqttCallBack);
}