#include "x_mqtt.h"


/* MQTT Pubclications *************************************************************************************/

mqttPublication m_mqttPubs[N_PUBS] = {
    {"error", 0, (mqttPubFunc)&mqttPublishAlert},
    {"admin", 0, (mqttPubFunc)&mqttPublishAdmin},
    {"state", 0, (mqttPubFunc)&mqttPublishState},
    {"config", 0, (mqttPubFunc)&mqttPublishConfig},

    {"ops", 0, (mqttPubFunc)&mqttPublishOps},
    {"ops/pos", 0, (mqttPubFunc)&mqttPublishOpsPosition},
};

void mqttPublishAlert(Alert* alert) { 

    char pTopic[MQTT_MAX_TOPIC] = SECRET_MQTT_DEVICE;
    mqttSIGBuilder(pTopic, m_mqttPubs[PUB_ERROR].topic);
    publishMQTTMessage(pTopic, (char *)alert->getJSON());
}

void mqttPublishAdmin() {
    
    char pTopic[MQTT_MAX_TOPIC] = SECRET_MQTT_DEVICE;
    mqttSIGBuilder(pTopic, m_mqttPubs[PUB_ADMIN].topic);
    publishMQTTMessage(pTopic, (char *)g_admin.serializeToJSON());
}

void mqttPublishState() { 
    
    char pTopic[MQTT_MAX_TOPIC] = SECRET_MQTT_DEVICE;
    mqttSIGBuilder(pTopic, m_mqttPubs[PUB_STATE].topic);
    publishMQTTMessage(pTopic, (char *)g_state.serializeToJSON());
}

void mqttPublishConfig() { 

    char pTopic[MQTT_MAX_TOPIC] = SECRET_MQTT_DEVICE;
    mqttSIGBuilder(pTopic, m_mqttPubs[PUB_CONFIG].topic);
    publishMQTTMessage(pTopic, (char *)g_config.serializeToJSON()); 
}


void mqttPublishOps() { 

    char pTopic[MQTT_MAX_TOPIC] = SECRET_MQTT_DEVICE;
    mqttSIGBuilder(pTopic, m_mqttPubs[PUB_OPS].topic);
    publishMQTTMessage(pTopic, (char *)g_ops.serializeToJSON()); 
}

void mqttPublishOpsPosition() {
    char buffer[20]; 
    snprintf(buffer, sizeof(buffer), "%.8f", g_state.currentHeight);

    char pTopic[MQTT_MAX_TOPIC] = SECRET_MQTT_DEVICE;
    mqttSIGBuilder(pTopic, m_mqttPubs[PUB_OPS_POS].topic);
    publishMQTTMessage(pTopic, buffer);
}


void setMQTTPubFlag(eMqttPubMap_t pub) {
    m_mqttPubs[pub].flag = 1;
}


/* MQTT Subscriptions *************************************************************************************/

/* DIAGNOSTIC COMMANDS ****************************************************************/

void mqttHandleCMDEnableDiagnostics(char* msg) { 
    g_ops.diagnosticMode = true; 
    mqttHandleCMDReport(msg);
}
void mqttHandleCMDDisableDiagnostics(char* msg) { 
    g_ops.diagnosticMode = false;
    mqttHandleCMDReport(msg); 
}

void mqttHandleCMDBrakeOn(char* msg) { 
    if( g_ops.diagnosticMode
    ) {
        brakeOn();
        mqttHandleCMDReport(msg);
    }
}
void mqttHandleCMDBrakeOff(char* msg) { 
    if( g_ops.diagnosticMode
    ) {
        brakeOff();
        mqttHandleCMDReport(msg);
    }
}

void mqttHandleCMDMagnetOn(char* msg) { 
    if( g_ops.diagnosticMode
    ) {
        magnetOn();
        mqttHandleCMDReport(msg);
    }
}
void mqttHandleCMDMagnetOff(char* msg) { 
    if( g_ops.diagnosticMode
    ) {
        magnetOff();
        mqttHandleCMDReport(msg);
    }
}


void mqttHandleCMDMoveUp(char* msg) { 
    if( g_ops.diagnosticMode
    ) {
        diagnosticMove(true);
        mqttHandleCMDReport(msg);
    }
}
void mqttHandleCMDMoveDown(char* msg) { 
    if( g_ops.diagnosticMode
    ) {
        diagnosticMove(false);
        mqttHandleCMDReport(msg);
    }
}
void diagnosticMove(bool up) {

    Alert* alert = nullptr;
    alert = motorSetSpeed(g_admin.motHzLow);
    if( alert
    )   mqttPublishAlert(alert);

    alert = motorGetPosition();         
    if( alert
    )   mqttPublishAlert(alert);

    // Serial.printf("\nCurrent position: %d\n", g_state.motorSteps);
    int32_t course = (up ? g_admin.diagSteps : g_admin.diagSteps * -1 );
    // Serial.printf("\nMoving: %d\n", course);

    motorSetCourse(course);
}

void mqttHandleCMDMotorStop(char* msg) { 
    if( g_ops.diagnosticMode
    ) {
        motorStop();
        mqttHandleCMDReport(msg);
    }
}


/* END DIAGNOSTIC COMMANDS ************************************************************/

mqttSubscription m_mqttSubs[N_SUBS] = {

    {"report", (mqttCMDFunc)&mqttHandleCMDReport},
    {"admin", (mqttCMDFunc)&mqttHandleCMDAdmin},
    {"state", (mqttCMDFunc)&mqttHandleCMDState},
    {"config", (mqttCMDFunc)&mqttHandleCMDConfig},

    {"ops", (mqttCMDFunc)&mqttHandleCMDOps},
    {"ops/reset", (mqttCMDFunc)&mqttHandleCMDOpsReset},
    {"ops/continue", (mqttCMDFunc)&mqttHandleCMDOpsContinue},

    {"diag/enable", (mqttCMDFunc)&mqttHandleCMDEnableDiagnostics},
    {"diag/disable", (mqttCMDFunc)&mqttHandleCMDDisableDiagnostics},

    {"diag/brake_on", (mqttCMDFunc)&mqttHandleCMDBrakeOn},
    {"diag/brake_off", (mqttCMDFunc)&mqttHandleCMDBrakeOff},

    {"diag/magnet_on", (mqttCMDFunc)&mqttHandleCMDMagnetOn},
    {"diag/magnet_off", (mqttCMDFunc)&mqttHandleCMDMagnetOff},

    {"diag/move_up", (mqttCMDFunc)&mqttHandleCMDMoveUp},
    {"diag/move_down", (mqttCMDFunc)&mqttHandleCMDMoveDown},
    {"diag/motor_stop", (mqttCMDFunc)&mqttHandleCMDMotorStop},
    
};


void mqttHandleCMDReport(char* msg) {
    setMQTTPubFlag(PUB_ADMIN);
    setMQTTPubFlag(PUB_CONFIG);
    setMQTTPubFlag(PUB_STATE);
    setMQTTPubFlag(PUB_OPS);
}

/* Admin */
void mqttHandleCMDAdmin(char* msg) {
    g_admin.parseFromJSON(msg);

    changeHammerTimeroutPeriod();
    changeITRDebounceTimerPeriod();
    setMQTTPubFlag(PUB_ADMIN);
}

/* State */
void mqttHandleCMDState(char* msg) { 
    setMQTTPubFlag(PUB_STATE);
}

/* Config */
void mqttHandleCMDConfig(char* msg) {
    g_config.parseFromJSON(msg); 
    g_ops.clearProgress();
    g_ops.goHome = true;   
    mqttHandleCMDReport(msg);
}

/* ops */
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
    // Serial.printf("\nMessage arrived on topic: %s", topic);
    for (mqttSubscription sub : m_mqttSubs) {
        
        char sTopic[MQTT_MAX_TOPIC] = SECRET_MQTT_DEVICE;
        mqttCMDBuilder(sTopic, sub.topic);
        if (strcmp(topic, sTopic) == 0) {
            // Serial.printf("\nSUB: %s\n", sTopic);
            sub.func((char*) message);
            break;
        }
    }
}

void setupMQTT_X(const char* mqttBrokerIP, int mqttBrokerPort) {
    setupMQTTClient(mqttBrokerIP, mqttBrokerPort,(mqttCallBackFunc)&mqttCallBack_X);
}

void serviceMQTTClient_X(const char* user, const char* pw) {
    
    if( g_state.interruptFlag //g_ui32InterruptFlag                     /* We have had a state change */
    ) {
        setMQTTPubFlag(PUB_STATE);              // We tell all of our state change
        g_state.interruptFlag = false; //g_ui32InterruptFlag = 0;                // We stop reacting to the state change, lest we look like fools!
    }

    serviceMQTTClient(user, pw, m_mqttSubs, N_SUBS);
    for (mqttPublication &pub : m_mqttPubs) {
        if ( pub.flag > 0 ) {
            // Serial.printf("\nPUB: %s\n", pub.topic);
            pub.func();
            pub.flag = 0;
        }
    }
}
