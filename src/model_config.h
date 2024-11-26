#ifndef MODEL_CONFIG_H
#define MODEL_CONFIG_H

#include <Arduino.h>
#include "dc_esp_server.h"
#include "dc_json.h"

class Config {
protected:
    const char* cyclesKey = "\"cycles\":";
    const char* heightKey = "\"height\":";
    
    char jsonOut[MQTT_PUB_BUFFER_SIZE];

public:
    int cycles;
    float height; 
    int steps;
    
    Config(
        int cyc=0, 
        float ht=0.0 
    ) {
        cycles = cyc;
        height = ht;
    }
    
    void parseFromJSON(const char* jsonString) {
        jsonParseInt(jsonString, cyclesKey, cycles);
        jsonParseFloat(jsonString, heightKey, height);
    }
    
    char* serializeToJSON() {
        jsonSerializeStart(jsonOut);
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
        Serial.printf("Cycles: %d\n", cycles);
        Serial.printf("Height: %f\n\n", height);
    }
};

#endif /* MODEL_CONFIG_H */