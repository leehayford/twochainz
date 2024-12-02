#ifndef MODEL_OPS_H
#define MODEL_OPS_H

#include <Arduino.h>
#include "dc_esp_server.h" 
#include "dc_json.h"

class Ops {
protected:

    const char* awaitEStopKey = "\"await_estop\":";
    const char* awaitDoorKey = "\"await_door\":";
    const char* awaitConfigKey = "\"await_config\":";
    const char* reqHelpKey = "\"req_help\":";
    const char* awaitHelpKey = "\"await_help\":";
    const char* recoveryKey = "\"recovery\":";
    
    const char* seekHammerKey = "\"seek_hammer\":";
    const char* seekAnvilKey = "\"seek_anvil\":";
    const char* seekHomeKey = "\"seek_home\":";

    const char* raiseHammerKey = "\"raise_hammer\":";
    const char* dropHammerKey = "\"drop_hammer\":";

    const char* cyclesCompletedKey = "\"cycles_completed\":";
    
    const char* statusKey = "\"status\":";

    char jsonOut[MQTT_PUB_BUFFER_SIZE];

public:

    bool awaitEStop;
    bool awaitDoor;
    bool awaitConfig;
    bool reqHelp;
    bool awaitHelp;
    bool recovery;

    bool seekHammer;
    bool seekAnvil;
    bool seekHome;

    bool raiseHammer;
    bool dropHammer;

    int cyclesCompleted;

    char status[JSON_FIELD_STRING_LENGTH];

    Ops(
        bool aw_es = false,
        bool aw_dr = false,
        bool aw_cfg = false,
        bool rq_hlp = false,
        bool aw_hlp = false,
        bool rec = false,
        
        bool sk_hmr = false,
        bool sk_anv = false,
        bool sk_hom = false,
        
        bool drp_hmr = false,
        bool ras_hmr = false,

        int cycles = 0,

        const char stat[JSON_FIELD_STRING_LENGTH]="initialized"
    ) {
        awaitEStop = aw_es;
        awaitDoor = aw_dr;
        awaitConfig = aw_cfg;
        reqHelp = rq_hlp;
        awaitHelp = aw_hlp;
        recovery = rec;

        seekHammer = sk_hmr;
        seekAnvil = sk_anv;
        seekHome = sk_hom;

        raiseHammer = ras_hmr;
        dropHammer = drp_hmr;

        cyclesCompleted = cycles;

        strcpy(status, stat);
    }
 
    void setStatus(const char stat[JSON_FIELD_STRING_LENGTH]) {
        strcpy(status, stat);
    }

    void clearProgress() {
        cyclesCompleted = 0;
        seekHammer = true;
        seekAnvil = true;
        seekHome = true;
    }

    void cmdReset() {
        clearProgress();
        awaitHelp = false;
    }

    void cmdContinue() {
        awaitHelp = false;
    }

    void parseFromJSON(const char* jsonString) {
        jsonParseBool(jsonString, awaitEStopKey, awaitEStop);
        jsonParseBool(jsonString, awaitDoorKey, awaitDoor);
        jsonParseBool(jsonString, awaitConfigKey, awaitConfig);
        jsonParseBool(jsonString, reqHelpKey, reqHelp);
        jsonParseBool(jsonString, awaitHelpKey, awaitHelp);
        jsonParseBool(jsonString, recoveryKey, recovery);

        jsonParseBool(jsonString, seekHammerKey, seekHammer);
        jsonParseBool(jsonString, seekAnvilKey, seekAnvil);
        jsonParseBool(jsonString, seekHomeKey, seekHome);

        jsonParseBool(jsonString, raiseHammerKey, raiseHammer);
        jsonParseBool(jsonString, dropHammerKey, dropHammer);

        jsonParseInt(jsonString, cyclesCompletedKey, cyclesCompleted);
        
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
        
        jsonSerializeBool(jsonOut, seekHammerKey, seekHammer);
        jsonSerializeBool(jsonOut, seekAnvilKey, seekAnvil); 
        jsonSerializeBool(jsonOut, seekHomeKey, seekHome); 

        jsonSerializeBool(jsonOut, raiseHammerKey, raiseHammer); 
        jsonSerializeBool(jsonOut, dropHammerKey, dropHammer);

        jsonSerializeInt(jsonOut, cyclesCompletedKey, cyclesCompleted);
        
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

        Serial.printf("Seek Hammer: %s\n", btoa(seekHammer));
        Serial.printf("Seek Anvil: %s\n", btoa(seekAnvil));
        Serial.printf("Seek Home: %s\n", btoa(seekHome));
        
        Serial.printf("Raise Hammer: %s\n", btoa(raiseHammer));
        Serial.printf("Drop Hammer: %s\n", btoa(dropHammer));

        Serial.printf("Cycles completed: %d\n", cyclesCompleted);

        Serial.printf("Status: %s\n\n", status);
    }
};

#endif /* MODEL_OPS_H */