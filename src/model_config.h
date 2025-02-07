#ifndef MODEL_CONFIG_H
#define MODEL_CONFIG_H

#include <Arduino.h>
#include "dc_esp_server.h"
#include "dc_json.h"
#include "model_admin.h"

class Config {
private:
    const char* cyclesKey = "\"cycles\":";
    const char* heightKey = "\"height\":";
    
    char jsonOut[MQTT_PUB_BUFFER_SIZE];

public:
    int cycles;
    float height; 
    
    Config() {
        cycles = 0;
        height = 0.0;
    }
    
    bool isValid() {
        return ( 
            cycles > 0                          /* We have a valid cycle setting */
        &&  height > 0
        &&  height > 0                          /* We have a valid height setting */
        &&  height < ADMIN_MOT_MAX_INCH     
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

        if( inch > ADMIN_MOT_MAX_INCH           /* We refuse to swing higher than this */
        )   inch = ADMIN_MOT_MAX_INCH;       

        return inch;
    }

    void resetConfig() {
        cycles = 0;
        height = 0.0;
    }
    
    void parseFromJSON(const char* jsonString) {

        try {
            int qty = 0;
            jsonParseInt(jsonString, cyclesKey, qty);
            cycles = validateCyces(qty);

            float inch = 0.0;
            jsonParseFloat(jsonString, heightKey, inch);
            height = validateHeight(inch);
        
        } catch (...) { 
            Serial.printf("\nmodel_config.parseFromJSON : FAILED \n");    
            throw -1;
        }
        
        Serial.printf("\nmodel_config.parseFromJSON : OK\n");
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