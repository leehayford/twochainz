#ifndef MODEL_OPS_H
#define MODEL_OPS_H

#include <Arduino.h>
#include "dc_esp_server.h" 
#include "dc_json.h"

#define STATUS_START "awakens"

#define STATUS_ESTOP "yearns for release... of the emergency stop button"
#define STATUS_DOOR_OPEN "yearns for closure... of the door"
#define STATUS_TOP_LIMIT "yearns to be less high... of fist"
#define STATUS_REQUEST_HELP "yearns for assistance"
#define STATUS_WANT_CONFIG "yearns for purpose"

#define STATUS_SEEK_HAMMER "seeks the hammer"
#define STATUS_SEEK_ANVIL "seeks the anvil"
#define STATUS_SEEK_HOME "seeks home"
#define STATUS_RAISE_HAMMER "raises the hammer"
#define STATUS_DROP_HAMMER "drops the hammer"

class Ops {
private:
    const char* diagnosticModeKey = "\"diagnostic_mode\":";

    const char* wantEStopReleaseKey = "\"want_estop_release\":";
    const char* wantDoorCloseKey = "\"want_door_close\":";
    const char* wantFistDownKey = "\"want_fist_down\":";
    const char* wantConfigKey = "\"want_config\":";

    const char* requestAidKey = "\"request_aid\":";
    const char* wantAidKey = "\"want_aid\":";
    const char* reorientKey = "\"reorient\":";

    const char* goHomeKey = "\"go_home\":";
    const char* seekHammerKey = "\"seek_hammer\":";
    const char* seekAnvilKey = "\"seek_anvil\":";
    const char* seekHomeKey = "\"seek_home\":";

    const char* raiseHammerKey = "\"raise_hammer\":";
    const char* dropHammerKey = "\"drop_hammer\":";
    const char* wantBrakeOffKey = "\"want_brake_off\":";
    const char* wantBrakeOnKey = "\"want_brake_on\":";
    const char* wantStrikeKey = "\"want_strike\":";

    const char* cycleCountKey = "\"cycle_count\":";
    const char* stepTargetKey = "\"step_target\":";
    const char* stepHzKey = "\"step_hz\":";
    
    const char* statusKey = "\"status\":";

    char jsonOut[MQTT_PUB_BUFFER_SIZE];

public:
    bool diagnosticMode;

    bool wantEStopRelease;
    bool wantDoorClose;
    bool wantFistDown;
    bool wantConfig;

    /* Cleared by Ops.doReorientation() */
    bool requestAid;
    /* Cleared by Ops.doReorientation() */
    bool wantAid;
    bool reorient;

    bool goHome;
    bool seekHammer;
    bool seekAnvil;
    bool seekHome;

    bool raiseHammer;
    bool dropHammer;
    bool wantBrakeOff;
    bool wantBrakeOn;
    bool wantStrike;

    int cycleCount;
    int stepTarget;
    int stepHz;

    char status[JSON_FIELD_STRING_LENGTH];

    Ops() {
        diagnosticMode = false;

        wantEStopRelease = false;
        wantDoorClose = false;
        wantFistDown = false;
        wantConfig = false;

        requestAid = false;
        wantAid = false;
        reorient = true;

        goHome = true;
        seekHammer = false;
        seekAnvil = false;
        seekHome = false;

        raiseHammer = false;
        dropHammer = false;
        wantBrakeOff = false;
        wantBrakeOn = false;
        wantStrike = false;

        cycleCount = 0;
        stepTarget = 0;
        stepHz = 0;

        setStatus("initialized");
    }
 
    void setStatus(const char stat[JSON_FIELD_STRING_LENGTH]) {
        strcpy(status, stat);
        Serial.printf("\ng_ops.setStatus( %s )\n", status);
    }

    void reassesOpStatus() {
        wantEStopRelease = false;
        wantDoorClose = false;
        wantFistDown = false;
        requestAid = false;
        wantConfig = false;
    }

    void doReorientation() {
        wantAid = false;                        // We stop yearning for aid, lest we look like fools!
        requestAid = false;                     // We stop requesting aid, lest we look like fools!
        
        reassesOpStatus();

        // These flags are cleared in the doGoHome function
        reorient = true;                        // We embark on a journey of self discovery
        goHome = true;                          // Spoiler alert: such journies nearly always lead us home
    }

    void clearProgress() {
        cycleCount = 0;
        stepTarget = 0;
    }

    void cmdReset() {
        clearProgress();
        doReorientation();
    }

    void cmdContinue() { // Serial.printf("Ops.cmdContinue() :\n");
        doReorientation();
    }

    /* TODO: MAKE READ ONLY AFTER DEBUG */
    void parseFromJSON(const char* jsonString) {
        jsonParseBool(jsonString, diagnosticModeKey, diagnosticMode);

        jsonParseBool(jsonString, wantEStopReleaseKey, wantEStopRelease);
        jsonParseBool(jsonString, wantDoorCloseKey, wantDoorClose);
        jsonParseBool(jsonString, wantFistDownKey, wantFistDown);
        jsonParseBool(jsonString, wantConfigKey, wantConfig);

        jsonParseBool(jsonString, requestAidKey, requestAid);
        jsonParseBool(jsonString, wantAidKey, wantAid);
        jsonParseBool(jsonString, reorientKey, reorient);

        jsonParseBool(jsonString, goHomeKey, goHome);
        jsonParseBool(jsonString, seekHammerKey, seekHammer);
        jsonParseBool(jsonString, seekAnvilKey, seekAnvil);
        jsonParseBool(jsonString, seekHomeKey, seekHome);

        jsonParseBool(jsonString, raiseHammerKey, raiseHammer);
        jsonParseBool(jsonString, dropHammerKey, dropHammer);
        jsonParseBool(jsonString, wantBrakeOffKey, wantBrakeOff);
        jsonParseBool(jsonString, wantBrakeOnKey, wantBrakeOn);
        jsonParseBool(jsonString, wantStrikeKey, wantStrike);

        jsonParseInt(jsonString, cycleCountKey, cycleCount);
        jsonParseInt(jsonString, stepTargetKey, stepTarget);
        jsonParseInt(jsonString, stepHzKey, stepHz);
        
        jsonParseString(jsonString, statusKey, status, sizeof(status));
    }
        
    char* serializeToJSON() {
        jsonSerializeStart(jsonOut);

        jsonSerializeBool(jsonOut, diagnosticModeKey, diagnosticMode);

        jsonSerializeBool(jsonOut, wantEStopReleaseKey, wantEStopRelease);
        jsonSerializeBool(jsonOut, wantDoorCloseKey, wantDoorClose);
        jsonSerializeBool(jsonOut, wantFistDownKey, wantFistDown);
        jsonSerializeBool(jsonOut, wantConfigKey, wantConfig);

        jsonSerializeBool(jsonOut, requestAidKey, requestAid);
        jsonSerializeBool(jsonOut, wantAidKey, wantAid);
        jsonSerializeBool(jsonOut, reorientKey, reorient);
        
        jsonSerializeBool(jsonOut, goHomeKey, goHome);
        jsonSerializeBool(jsonOut, seekHammerKey, seekHammer);
        jsonSerializeBool(jsonOut, seekAnvilKey, seekAnvil); 
        jsonSerializeBool(jsonOut, seekHomeKey, seekHome); 

        jsonSerializeBool(jsonOut, raiseHammerKey, raiseHammer); 
        jsonSerializeBool(jsonOut, dropHammerKey, dropHammer);
        jsonSerializeBool(jsonOut, wantBrakeOffKey, wantBrakeOff);
        jsonSerializeBool(jsonOut, wantBrakeOnKey, wantBrakeOn);
        jsonSerializeBool(jsonOut, wantStrikeKey, wantStrike);

        jsonSerializeInt(jsonOut, cycleCountKey, cycleCount);
        jsonSerializeInt(jsonOut, stepTargetKey, stepTarget);
        jsonSerializeInt(jsonOut, stepHzKey, stepHz);
        
        jsonSerializeString(jsonOut, statusKey, status); 

        jsonSerializeEnd(jsonOut);
        return jsonOut;
    }
    
    void debugPrintJSON() {
        Serial.printf("Ops.debugPrintJSON() :\n%s\n\n", jsonOut);
    }
};

#endif /* MODEL_OPS_H */