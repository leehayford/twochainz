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
    const char* cyclesKey = "\"cycles\":";
    const char* heightKey = "\"height\":";
    
    char jsonOut[MQTT_PUB_BUFFER_SIZE];

public:
    int cycles;
    float height; 
    int steps;
    
    Config() {
        cycles = 0;
        height = 0.0;
    }
    
    bool isValid() {
        return ( 
            cycles > 0                          /* We have a valid cycle setting */
        &&  height > 0
        &&  height > 0                          /* We have a valid height setting */
        &&  height < FIST_HEIGHT_MAX_INCH       
        );
    }

    int validateCyces(int qty) {
        if( qty < 0                             /* We refuse to undo cycles */
        )   qty = 0;                            

        return qty;
    }

    float validateHeight(float inch) {

        if( inch < 0                            /* The refuse to push through the anvli */
        )   inch == 0.0;                        

        if( inch > FIST_HEIGHT_MAX_INCH         /* We refuse to swing higher than this */
        )   inch = FIST_HEIGHT_MAX_INCH;        

        return inch;
    }

    void resetConfig() {
        cycles = 0;
        height = 0.0;
    }
    
    void parseFromJSON(const char* jsonString) {

        int qty = 0;
        jsonParseInt(jsonString, cyclesKey, qty);
        cycles = validateCyces(qty);

        float inch = 0.0;
        jsonParseFloat(jsonString, heightKey, inch);
        height = validateHeight(inch);

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

};

#endif /* MODEL_CONFIG_H */