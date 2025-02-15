#include "x_mqtt.h"

/* MQTT Pubclications *************************************************************************************/

mqttPublication m_mqttPubs[N_PUBS] = {
    {"alert", 0, (mqttPubFunc)&mqttPublishAlert},
    
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

void diagnosticReoprt() {
    setMQTTPubFlag(PUB_STATE);
    setMQTTPubFlag(PUB_OPS);
}

void opsStatusReport(const char* status_msg) { 
    g_ops.setStatus(status_msg);
    setMQTTPubFlag(PUB_CONFIG);
    setMQTTPubFlag(PUB_STATE);
    setMQTTPubFlag(PUB_OPS);
}

/* MQTT Subscriptions *************************************************************************************/


mqttSubscription m_mqttSubs[N_SUBS] = {

    {"report", (mqttCMDFunc)&mqttHandleCMDReport},
    
    {"admin", (mqttCMDFunc)&mqttHandleCMDAdmin},
    {"admin/set_def", (mqttCMDFunc)&mqttHandleCMDAdminSetDefaults},
    {"admin/get_def", (mqttCMDFunc)&mqttHandleCMDAdminGetDefaults},
    
    {"state", (mqttCMDFunc)&mqttHandleCMDState},
    
    {"config", (mqttCMDFunc)&mqttHandleCMDConfig},

    {"ops", (mqttCMDFunc)&mqttHandleCMDOps},
    {"ops/reset", (mqttCMDFunc)&mqttHandleCMDOpsReset},
    {"ops/run", (mqttCMDFunc)&mqttHandleCMDOpsRun},
    {"ops/pause", (mqttCMDFunc)&mqttHandleCMDOpsPause},
    {"ops/continue", (mqttCMDFunc)&mqttHandleCMDOpsContinue},
    {"ops/en_awes", (mqttCMDFunc)&mqttHandleCMDEnableAWESMode},
    {"ops/dis_awes", (mqttCMDFunc)&mqttHandleCMDDisableAWESMode},

    {"diag/enable", (mqttCMDFunc)&mqttHandleCMDEnableDiagnostics},
    {"diag/disable", (mqttCMDFunc)&mqttHandleCMDDisableDiagnostics},

    {"diag/brake_on", (mqttCMDFunc)&mqttHandleCMDBrakeOn},
    {"diag/brake_off", (mqttCMDFunc)&mqttHandleCMDBrakeOff},

    {"diag/magnet_on", (mqttCMDFunc)&mqttHandleCMDMagnetOn},
    {"diag/magnet_off", (mqttCMDFunc)&mqttHandleCMDMagnetOff},

    {"diag/move_up", (mqttCMDFunc)&mqttHandleCMDMoveUp},
    {"diag/move_down", (mqttCMDFunc)&mqttHandleCMDMoveDown},
    
    {"diag/motor_stop", (mqttCMDFunc)&mqttHandleCMDMotorStop},
    {"diag/motor_zero", (mqttCMDFunc)&mqttHandleCMDMotorZero},
    
};

void mqttHandleCMDReport(char* msg) {
    setMQTTPubFlag(PUB_ADMIN);
    setMQTTPubFlag(PUB_CONFIG);
    setMQTTPubFlag(PUB_STATE);
    setMQTTPubFlag(PUB_OPS);
}

/* Admin */
void mqttHandleCMDAdmin(char* msg) {
    validateAdminSettings(msg);
    setMQTTPubFlag(PUB_ADMIN);
}
void mqttHandleCMDAdminSetDefaults(char* msg) {
    validateAdminSettings(msg);
    writeAdminSettingsToFile();
}
void mqttHandleCMDAdminGetDefaults(char* msg) {
    validateAdminSettingsFile();
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
    setMQTTPubFlag(PUB_CONFIG);
    setMQTTPubFlag(PUB_OPS);
}

/* ops */
void mqttHandleCMDOps(char* msg) {
    setMQTTPubFlag(PUB_OPS);
}

void mqttHandleCMDOpsReset(char* msg) {
    g_config.resetConfig();                    
    g_ops.resetOps();    
    motorStop();
    mqttHandleCMDReport(msg);           // And if they don't know, now they know
}

void mqttHandleCMDOpsRun(char* msg) {
    if( g_config.isValid()              /* Our configuration is valid */
    ) {  
        if( g_ops.cycleCount == g_config.cycles
        )   g_ops.clearProgress();
        g_ops.runOps();                 // We begin / resume our quest
        g_ops.setStatus("running...");
    }

    mqttHandleCMDReport(msg);           // And if they don't know, now they know
}

void mqttHandleCMDOpsPause(char* msg) {
    g_ops.pauseOps();                   /* Unconditionally we pause */
    motorStop();
    mqttHandleCMDReport(msg);           // And if they don't know, now they know
}

void mqttHandleCMDOpsContinue(char* msg) {
    g_ops.continueOps();                /* Unconditionally we pause */

    mqttHandleCMDReport(msg);           // And if they don't know, now they know
}

void mqttHandleCMDEnableAWESMode(char* msg) {
    g_ops.awesMode = true;
    g_config.cycles = 1;
    mqttHandleCMDReport(msg);
}

void mqttHandleCMDDisableAWESMode(char* msg) {
    g_ops.awesMode = false;
    mqttHandleCMDReport(msg);
}
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
        diagnosticReoprt();
    }
}
void mqttHandleCMDBrakeOff(char* msg) { 
    if( g_ops.diagnosticMode
    ) {
        brakeOff();
        diagnosticReoprt();
    }
}

void mqttHandleCMDMagnetOn(char* msg) { 
    if( g_ops.diagnosticMode
    ) {
        magnetOn();
        diagnosticReoprt();
    }
}
void mqttHandleCMDMagnetOff(char* msg) { 
    if( g_ops.diagnosticMode
    ) {
        magnetOff();
        diagnosticReoprt();
    }
}

void diagnosticMove(bool up) {

    motorGetPosition(); 
    motorSetSpeed(g_admin.motHzLow);        
    motorSetCourse((up 
        ? g_admin.diagSteps             // move UP (+)  
        : g_admin.diagSteps * -1        // move DOWN (-) 
    ));
}
void mqttHandleCMDMoveUp(char* msg) { 
    if( g_ops.diagnosticMode
    ) {
        diagnosticMove(true);
        diagnosticReoprt();
    }
}
void mqttHandleCMDMoveDown(char* msg) { 
    if( g_ops.diagnosticMode
    ) {
        diagnosticMove(false);
        diagnosticReoprt();
    }
}

void mqttHandleCMDMotorStop(char* msg) { 
    if( g_ops.diagnosticMode
    ) {
        motorStop();
        diagnosticReoprt();
    }
}
void mqttHandleCMDMotorZero(char* msg) {
    if( g_ops.diagnosticMode
    ) {
        motorSetPositionAsZero();
        diagnosticReoprt();
    }
}

/* END DIAGNOSTIC COMMANDS ************************************************************/



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
    
    if( g_state.stateChangeFlag             /* We have had a hardware state change */
    ) {
        g_state.stateChangeFlag = false;    // We stop reacting to the state change, lest we look like fools!
        setMQTTPubFlag(PUB_STATE);          // We tell all of our state change
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
