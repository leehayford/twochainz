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
        bool ras_hmr = false
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
    };

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

        jsonSerializeEnd(jsonOut);
        return jsonOut;
    }
    
    void debugPrintJSON() {
        Serial.printf("Ops.debugPrintJSON() :\n%s\n\n", jsonOut);
    }
};

#endif /* MODEL_OPS_H */