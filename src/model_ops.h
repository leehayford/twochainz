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
    const char* reqHelpKey = "\"req_help\":";
    const char* awaitHelpKey = "\"await_help\":";
    const char* recoveryKey = "\"recovery\":";
    
    const char* awaitStartKey = "\"await_start\":";

    const char* goHomeKey = "\"go_home\":";
    const char* awaitHammerKey = "\"await_hammer\":";
    const char* awaitAnvilKey = "\"await_anvil\":";
    const char* awaitHomeKey = "\"await_home\":";

    const char* raiseHammerKey = "\"raise_hammer\":";
    const char* dropHammerKey = "\"drop_hammer\":";
    const char* awaitDropKey = "\"await_drop\":";

    const char* cycleCountKey = "\"cycle_count\":";
    const char* stepTargetKey = "\"step_target\":";
    const char* stepHzKey = "\"step_hz\":";
    
    const char* statusKey = "\"status\":";

    char jsonOut[MQTT_PUB_BUFFER_SIZE];

public:

    bool awaitEStop;
    bool awaitDoor;
    bool awaitConfig;
    bool reqHelp;
    bool awaitHelp;
    bool recovery;

    bool awaitStart;

    bool goHome;
    bool awaitHammer;
    bool awaitAnvil;
    bool awaitHome;

    bool raiseHammer;
    bool dropHammer;
    bool awaitDrop;

    int cycleCount;
    int stepTarget;
    int stepHz;

    char status[JSON_FIELD_STRING_LENGTH];

    Ops(
        bool aw_es = false,
        bool aw_dr = false,
        bool aw_cfg = false,
        bool rq_hlp = false,
        bool aw_hlp = false,
        bool rec = false,
        
        bool aw_start = false,

        bool go_home = false,
        bool aw_hmr = false,
        bool aw_anv = false,
        bool aw_hom = false,
        
        bool ras_hmr = false,
        bool drp_hmr = false,
        bool aw_drp = false,

        int cycles = 0,
        int target = 0,
        int hz = 0,

        const char stat[JSON_FIELD_STRING_LENGTH]="initialized"
    ) {
        awaitEStop = aw_es;
        awaitDoor = aw_dr;
        awaitConfig = aw_cfg;
        reqHelp = rq_hlp;
        awaitHelp = aw_hlp;
        recovery = rec;

        awaitStart = aw_start;

        goHome = go_home;
        awaitHammer = aw_hmr;
        awaitAnvil = aw_anv;
        awaitHome = aw_hom;

        raiseHammer = ras_hmr;
        dropHammer = drp_hmr;
        awaitDrop = aw_drp;

        cycleCount = cycles;
        stepTarget = target;
        stepHz = hz;

        strcpy(status, stat);
    }
 
    void setStatus(const char stat[JSON_FIELD_STRING_LENGTH]) {
        strcpy(status, stat);
    }

    void clearProgress() {
        cycleCount = 0;
    }

    void cmdReset() {
        clearProgress();
        awaitHelp = false;
    }

    void cmdContinue() {
        awaitHelp = false;
    }

    /* TODO: MAKE READ ONLY AFTER DEBUG */
    void parseFromJSON(const char* jsonString) {
        jsonParseBool(jsonString, awaitEStopKey, awaitEStop);
        jsonParseBool(jsonString, awaitDoorKey, awaitDoor);
        jsonParseBool(jsonString, awaitConfigKey, awaitConfig);
        jsonParseBool(jsonString, reqHelpKey, reqHelp);
        jsonParseBool(jsonString, awaitHelpKey, awaitHelp);
        jsonParseBool(jsonString, recoveryKey, recovery);

        jsonParseBool(jsonString, awaitStartKey, awaitStart);

        jsonParseBool(jsonString, goHomeKey, goHome);
        jsonParseBool(jsonString, awaitHammerKey, awaitHammer);
        jsonParseBool(jsonString, awaitAnvilKey, awaitAnvil);
        jsonParseBool(jsonString, awaitHomeKey, awaitHome);

        jsonParseBool(jsonString, raiseHammerKey, raiseHammer);
        jsonParseBool(jsonString, dropHammerKey, dropHammer);
        jsonParseBool(jsonString, awaitDropKey, awaitDrop);

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
        jsonSerializeBool(jsonOut, reqHelpKey, reqHelp);
        jsonSerializeBool(jsonOut, awaitHelpKey, awaitHelp);
        jsonSerializeBool(jsonOut, recoveryKey, recovery);
        
        jsonSerializeBool(jsonOut, awaitStartKey, awaitStart);
        
        jsonSerializeBool(jsonOut, goHomeKey, goHome);
        jsonSerializeBool(jsonOut, awaitHammerKey, awaitHammer);
        jsonSerializeBool(jsonOut, awaitAnvilKey, awaitAnvil); 
        jsonSerializeBool(jsonOut, awaitHomeKey, awaitHome); 

        jsonSerializeBool(jsonOut, raiseHammerKey, raiseHammer); 
        jsonSerializeBool(jsonOut, dropHammerKey, dropHammer);
        jsonSerializeBool(jsonOut, awaitDropKey, awaitDrop);

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
        Serial.printf("Request Help: %s\n", btoa(reqHelp));
        Serial.printf("Await Help: %s\n", btoa(awaitHelp));
        Serial.printf("Recovery Mode: %s\n", btoa(recovery));

        Serial.printf("Await Start: %s\n", btoa(awaitStart));

        Serial.printf("Go Home: %s\n", btoa(goHome));
        Serial.printf("Await Hammer: %s\n", btoa(awaitHammer));
        Serial.printf("Await Anvil: %s\n", btoa(awaitAnvil));
        Serial.printf("Await Home: %s\n", btoa(awaitHome));
        
        Serial.printf("Raise Hammer: %s\n", btoa(raiseHammer));
        Serial.printf("Drop Hammer: %s\n", btoa(dropHammer));

        Serial.printf("Cycle Count: %d\n", cycleCount);
        Serial.printf("Step Target: %d\n", stepTarget);
        Serial.printf("Step Hz: %d\n", stepHz);

        Serial.printf("Status: %s\n\n", status);
    }
};

#endif /* MODEL_OPS_H */