#ifndef MODEL_OPS_H
#define MODEL_OPS_H

#include <Arduino.h>
#include "dc_esp_server.h" 
#include "dc_json.h"

class Ops {
private:

    const char* awaitEStopKey = "\"await_estop\":";
    const char* awaitDoorKey = "\"await_door\":";
    const char* awaitConfigKey = "\"await_config\":";

    const char* seekHelpKey = "\"seek_help\":";
    const char* awaitHelpKey = "\"await_help\":";
    const char* recoveryKey = "\"recovery\":";

    const char* goHomeKey = "\"go_home\":";
    const char* seekHammerKey = "\"seek_hammer\":";
    const char* seekAnvilKey = "\"seek_anvil\":";
    const char* seekHomeKey = "\"seek_home\":";

    const char* raiseHammerKey = "\"raise_hammer\":";
    const char* dropHammerKey = "\"drop_hammer\":";
    const char* awaitStrikeKey = "\"await_strike\":";

    const char* cycleCountKey = "\"cycle_count\":";
    const char* stepTargetKey = "\"step_target\":";
    const char* stepHzKey = "\"step_hz\":";
    
    const char* statusKey = "\"status\":";

    char jsonOut[MQTT_PUB_BUFFER_SIZE];

public:

    bool awaitEStop;
    bool awaitDoor;
    bool awaitConfig;

    bool seekHelp;
    bool awaitHelp;
    bool recovery;

    bool goHome;
    bool seekHammer;
    bool seekAnvil;
    bool seekHome;

    bool raiseHammer;
    bool dropHammer;
    bool awaitStrike;

    int cycleCount;
    int stepTarget;
    int stepHz;

    char status[JSON_FIELD_STRING_LENGTH];

    Ops(
        bool aw_es = false,
        bool aw_dr = false,
        bool aw_cfg = false,

        bool sk_hlp = false,
        bool aw_hlp = false,
        bool rec = false,

        bool go_home = false,
        bool sk_hmr = false,
        bool sk_anv = false,
        bool sk_hom = false,
        
        bool ras_hmr = false,
        bool drp_hmr = false,
        bool aw_strk = false,

        int cycles = 0,
        int target = 0,
        int hz = 0,

        const char stat[JSON_FIELD_STRING_LENGTH]="initialized"
    ) {
        awaitEStop = aw_es;
        awaitDoor = aw_dr;
        awaitConfig = aw_cfg;

        seekHelp = sk_hlp;
        awaitHelp = aw_hlp;
        recovery = rec;

        goHome = go_home;
        seekHammer = sk_hmr;
        seekAnvil = sk_anv;
        seekHome = sk_hom;

        raiseHammer = ras_hmr;
        dropHammer = drp_hmr;
        awaitStrike = aw_strk;

        cycleCount = cycles;
        stepTarget = target;
        stepHz = hz;

        strcpy(status, stat);
    }
 
    void setStatus(const char stat[JSON_FIELD_STRING_LENGTH]) {
        strcpy(status, stat);
    }

    void beginRecovery() {
        awaitHelp = false;                      // We stop awaiting help, lest we look like fools!
        seekHelp = false;                       // We stop requesting help, lest we look like fools!
        // These flags are cleared in the doGoHome function
        recovery = true;                        // We must be cautious 
        goHome = true;                          // We must go home 
    }

    void clearProgress() {
        cycleCount = 0;
    }

    void cmdReset() {
        clearProgress();
        beginRecovery();
    }

    void cmdContinue() {
        beginRecovery();
    }

    /* TODO: MAKE READ ONLY AFTER DEBUG */
    void parseFromJSON(const char* jsonString) {
        jsonParseBool(jsonString, awaitEStopKey, awaitEStop);
        jsonParseBool(jsonString, awaitDoorKey, awaitDoor);
        jsonParseBool(jsonString, awaitConfigKey, awaitConfig);

        jsonParseBool(jsonString, seekHelpKey, seekHelp);
        jsonParseBool(jsonString, awaitHelpKey, awaitHelp);
        jsonParseBool(jsonString, recoveryKey, recovery);

        jsonParseBool(jsonString, goHomeKey, goHome);
        jsonParseBool(jsonString, seekHammerKey, seekHammer);
        jsonParseBool(jsonString, seekAnvilKey, seekAnvil);
        jsonParseBool(jsonString, seekHomeKey, seekHome);

        jsonParseBool(jsonString, raiseHammerKey, raiseHammer);
        jsonParseBool(jsonString, dropHammerKey, dropHammer);
        jsonParseBool(jsonString, awaitStrikeKey, awaitStrike);

        jsonParseInt(jsonString, cycleCountKey, cycleCount);
        jsonParseInt(jsonString, stepTargetKey, stepTarget);
        jsonParseInt(jsonString, stepHzKey, stepHz);
        
        jsonParseString(jsonString, statusKey, status, sizeof(status));
    }
        
    char* serializeToJSON() {
        jsonSerializeStart(jsonOut);

        jsonSerializeBool(jsonOut, awaitEStopKey, awaitEStop);
        jsonSerializeBool(jsonOut, awaitDoorKey, awaitDoor);
        jsonSerializeBool(jsonOut, awaitConfigKey, awaitConfig);

        jsonSerializeBool(jsonOut, seekHelpKey, seekHelp);
        jsonSerializeBool(jsonOut, awaitHelpKey, awaitHelp);
        jsonSerializeBool(jsonOut, recoveryKey, recovery);
        
        jsonSerializeBool(jsonOut, goHomeKey, goHome);
        jsonSerializeBool(jsonOut, seekHammerKey, seekHammer);
        jsonSerializeBool(jsonOut, seekAnvilKey, seekAnvil); 
        jsonSerializeBool(jsonOut, seekHomeKey, seekHome); 

        jsonSerializeBool(jsonOut, raiseHammerKey, raiseHammer); 
        jsonSerializeBool(jsonOut, dropHammerKey, dropHammer);
        jsonSerializeBool(jsonOut, awaitStrikeKey, awaitStrike);

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

        Serial.printf("Await EStop Clear: %s\n", btoa(awaitEStop));
        Serial.printf("Await Door Close: %s\n", btoa(awaitDoor));
        Serial.printf("Await Configuration: %s\n", btoa(awaitConfig));

        Serial.printf("Request Help: %s\n", btoa(seekHelp));
        Serial.printf("Await Help: %s\n", btoa(awaitHelp));
        Serial.printf("Recovery Mode: %s\n", btoa(recovery));

        Serial.printf("Go Home: %s\n", btoa(goHome));
        Serial.printf("Seek Hammer: %s\n", btoa(seekHammer));
        Serial.printf("Seek Anvil: %s\n", btoa(seekAnvil));
        Serial.printf("Seek Home: %s\n", btoa(seekHome));
        
        Serial.printf("Raise Hammer: %s\n", btoa(raiseHammer));
        Serial.printf("Drop Hammer: %s\n", btoa(dropHammer));
        Serial.printf("Await Strike: %s\n", btoa(awaitStrike));

        Serial.printf("Cycle Count: %d\n", cycleCount);
        Serial.printf("Step Target: %d\n", stepTarget);
        Serial.printf("Step Hz: %d\n", stepHz);

        Serial.printf("Status: %s\n\n", status);
    }
};

#endif /* MODEL_OPS_H */