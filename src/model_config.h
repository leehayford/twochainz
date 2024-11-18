#ifndef MODEL_CONFIG_H
#define MODEL_CONFIG_H

#include <Arduino.h>
#include "dc_json.h"

class Config {
protected:
    const char* isLoadedKey = "\"isloaded\":";
    const char* cyclesKey = "\"cycles\":";
    const char* heightKey = "\"height\":";
    const char* statusKey = "\"status\":";
    char jsonOut[JSON_OBJECT_SERIALIZED_LENGTH];
    
public:
    bool isLoaded; 
    int cycles;
    float height; 
    char status[JSON_FIELD_STRING_LENGTH];
    
    Config(
        bool l=false, 
        int c=0, 
        float h=0.0, 
        const char s[JSON_FIELD_STRING_LENGTH]="initialized"
    ) {
        isLoaded = l;
        cycles = c;
        height = h;
        strcpy(status, s);
    }
    
    void setStatus(const char s[JSON_FIELD_STRING_LENGTH]) {
        strcpy(status, s);
    }
    
    void parseJSONToConfig(const char* jsonString) {
        jsonParseBool(jsonString, isLoadedKey, isLoaded);
        jsonParseInt(jsonString, cyclesKey, cycles);
        jsonParseFloat(jsonString, heightKey, height);
        jsonParseString(jsonString, statusKey, status, sizeof(status));
    }
    
    char* serializeConfigToJSON() {
        jsonSerializeStart(jsonOut);
        jsonSerializeBool(jsonOut, isLoadedKey, isLoaded);
        jsonSerializeInt(jsonOut, cyclesKey, cycles);
        jsonSerializeFloat(jsonOut, heightKey, height);
        jsonSerializeString(jsonOut, statusKey, status);
        jsonSerializeEnd(jsonOut);
        return jsonOut;
    }
    
    void debugPrintJSON() {
        // printf("Config.debugPrintJSON() :\n%s\n\n", jsonOut);

        Serial.printf("Config.debugPrintJSON() :\n%s\n\n", jsonOut);
    }
    
    void debugPrintValues() {
        // printf("Config.debugPrintValues() :\n");
        // printf("Is loaded: %s\n", btoa(isLoaded));
        // printf("Cycles: %d\n", cycles);
        // printf("Height: %f\n", height);
        // printf("Status: %s\n\n", status);
        
        Serial.printf("Config.debugPrintValues() :\n");
        Serial.printf("Is loaded: %s\n", btoa(isLoaded));
        Serial.printf("Cycles: %d\n", cycles);
        Serial.printf("Height: %f\n", height);
        Serial.printf("Status: %s\n\n", status);
    }
};


/* EXAMPLE 
    Config cfg;
    cfg.debugPrintValues();
    
    const char* jsonString = R"({
        "isloaded": true,
        "cycles": 30,
        "height": 5.5,
        "status": "Configuration set"
    })";
    
    cfg.parseJSONToConfig(jsonString);
    cfg.serializeConfigToJSON();
    cfg.debugPrintJSON();
    
    cfg.cycles = 27;
    cfg.height = 38.75;
    cfg.setStatus("Configuration altered");
    
    jsonString = cfg.serializeConfigToJSON();
    cfg.debugPrintJSON();
    
    cfg.parseJSONToConfig(jsonString);
    cfg.debugPrintValues();
    
*/

#endif /* MODEL_CONFIG_H */