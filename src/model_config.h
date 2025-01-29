#ifndef MODEL_CONFIG_H
#define MODEL_CONFIG_H

#include <Arduino.h>
#include "dc_esp_server.h"
#include "dc_json.h"

#define FIST_INCH_PER_REV 6.000
#define FIST_HEIGHT_MAX_INCH 48.000
#define FIST_HEIGHT_MAX_STEP 16000 // = ( 48 / 6 ) * 2000

class Config {
private:
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
    
    bool validate() {
        return ( 
            run                                 /* We have been told to run */
        &&  cycles > 0                          /* We have a valid cycle setting */
        &&  height > 0
        &&  height > 0                          /* We have a valid height setting */
        &&  height < FIST_HEIGHT_MAX_INCH       
        );
    }

    void cmdReset() {
        run = false;
        cycles = 0;
        height = 0.0;
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

};

#endif /* MODEL_CONFIG_H */