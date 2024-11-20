#ifndef MODEL_CONFIG_H
#define MODEL_CONFIG_H

#include <Arduino.h>
#include "dc_json.h"

class Config {
protected:
    const char* cyclesKey = "\"cycles\":";
    const char* heightKey = "\"height\":";
    
    char jsonOut[JSON_OBJECT_SERIALIZED_LENGTH];

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


/* EXAMPLE 
    Config cfg;
    cfg.debugPrintValues();
    
    const char* jsonString = R"({
        "cycles": 30,
        "height": 5.5
    })";
    
    cfg.parseFromJSON(jsonString);
    cfg.serializeToJSON();
    cfg.debugPrintJSON();
    
    cfg.cycles = 27;
    cfg.height = 38.75;
    
    jsonString = cfg.serializeToJSON();
    cfg.debugPrintJSON();
    
    cfg.parseFromJSON(jsonString);
    cfg.debugPrintValues();
    
*/

#endif /* MODEL_CONFIG_H */