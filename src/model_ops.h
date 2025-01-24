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

    bool requestAid;
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

    Ops(
        bool diag_mode = false,

        bool wnt_es = false,
        bool wnt_dr = false,
        bool wnt_fd = false,
        bool wnt_cfg = false,

        bool req_aid = false,
        bool wnt_aid = false,
        bool reo = false,

        bool go_home = false,
        bool sk_hmr = false,
        bool sk_anv = false,
        bool sk_hom = false,
        
        bool ras_hmr = false,
        bool drp_hmr = false,
        bool wnt_strk = false,

        int cyc_cnt = 0,
        int stp_trg = 0,
        int stp_hz = 0,

        const char stat[JSON_FIELD_STRING_LENGTH]="initialized"
    ) {
        diagnosticMode = diag_mode;

        wantEStopRelease = wnt_es;
        wantDoorClose = wnt_dr;
        wantFistDown = wnt_fd;
        wantConfig = wnt_cfg;

        requestAid = req_aid;
        wantAid = wnt_aid;
        reorient = reo;

        goHome = go_home;
        seekHammer = sk_hmr;
        seekAnvil = sk_anv;
        seekHome = sk_hom;

        raiseHammer = ras_hmr;
        dropHammer = drp_hmr;
        wantStrike = wnt_strk;

        cycleCount = cyc_cnt;
        stepTarget = stp_trg;
        stepHz = stp_hz;

        strcpy(status, stat);
    }
 
    void setStatus(const char stat[JSON_FIELD_STRING_LENGTH]) {
        strcpy(status, stat);
        Serial.printf("\ng_ops.setStatus( %s )\n", status);
    }

    void doReorientation() {
        wantAid = false;                        // We stop yearning for aid, lest we look like fools!
        requestAid = false;                     // We stop requesting aid, lest we look like fools!
        
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

    void cmdContinue() {
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

    void debugPrintValues() {
        Serial.printf("Ops.debugPrintValues() :\n");

        Serial.printf("DIAGNOSTIC MODE: %s\n", btoa(diagnosticMode));

        Serial.printf("Want EStop Release: %s\n", btoa(wantEStopRelease));
        Serial.printf("Want Door Closed: %s\n", btoa(wantDoorClose));
        Serial.printf("Want Fist Down: %s\n", btoa(wantFistDown));
        Serial.printf("Want Configuration: %s\n", btoa(wantConfig));

        Serial.printf("Request Aid: %s\n", btoa(requestAid));
        Serial.printf("Want Aid: %s\n", btoa(wantAid));
        Serial.printf("Reorient: %s\n", btoa(reorient));

        Serial.printf("Go Home: %s\n", btoa(goHome));
        Serial.printf("Seek Hammer: %s\n", btoa(seekHammer));
        Serial.printf("Seek Anvil: %s\n", btoa(seekAnvil));
        Serial.printf("Seek Home: %s\n", btoa(seekHome));
        
        Serial.printf("Raise Hammer: %s\n", btoa(raiseHammer));
        Serial.printf("Drop Hammer: %s\n", btoa(dropHammer));
        Serial.printf("Want Hammer Strike: %s\n", btoa(wantStrike));

        Serial.printf("Cycle Count: %d\n", cycleCount);
        Serial.printf("Step Target: %d\n", stepTarget);
        Serial.printf("Step Hz: %d\n", stepHz);

        Serial.printf("Status: %s\n\n", status);
    }
};

#endif /* MODEL_OPS_H */