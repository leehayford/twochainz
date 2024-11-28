#ifndef MODEL_CONFIG_H
#define MODEL_CONFIG_H

#include <Arduino.h>
#include "dc_esp_server.h"
#include "dc_json.h"

class Config {
protected:
    const char* runKey = "\"run\":";
    const char* cyclesKey = "\"cycles\":";
    const char* heightKey = "\"height\":";
    
    char jsonOut[MQTT_PUB_BUFFER_SIZE];

public:
    bool run;
    int cycles;
    float height; 
    int steps;
    
    Config(
        bool r=false,
        int cyc=0, 
        float ht=0.0 
    ) {
        run = r;
        cycles = cyc;
        height = ht;
    }
    
    void parseFromJSON(const char* jsonString) {
        jsonParseBool(jsonString, runKey, run);
        jsonParseInt(jsonString, cyclesKey, cycles);
        jsonParseFloat(jsonString, heightKey, height);
    }
    
    char* serializeToJSON() {
        jsonSerializeStart(jsonOut);
        jsonSerializeBool(jsonOut, runKey, run);
        jsonSerializeInt(jsonOut, cyclesKey, cycles);
        jsonSerializeFloat(jsonOut, heightKey, height);
        jsonSerializeEnd(jsonOut);
        return jsonOut;
    }
    
    void debugPrintJSON() {
        Serial.printf("Config.debugPrintJSON() :\n%s\n\n", jsonOut);
    }
    
    void debugPrintValues() {
        Serial.printf("Config.debugPrintValues() :\n");
        Serial.printf("Run: %s\n", btoa(run));
        Serial.printf("Cycles: %d\n", cycles);
        Serial.printf("Height: %f\n\n", height);
    }
};

#endif /* MODEL_CONFIG_H */