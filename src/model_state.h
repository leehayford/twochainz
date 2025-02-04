#ifndef MODEL_STATE_H
#define MODEL_STATE_H

#include <Arduino.h>
#include "dc_esp_server.h"
#include "dc_json.h"

class State {
private:

    const char* eStopKey = "\"estop\":";
    const char* doorOpenKey = "\"door_open\":";
    const char* fistLimitKey = "\"fist_limit\":";
    const char* anvilLimitKey = "\"anvil_limit\":";
    const char* homeLimitKey = "\"home_limit\":";
    const char* topLimitKey = "\"top_limit\":";
    const char* pressureKey = "\"pressure\":";

    const char* brakeOnKey = "\"brake_on\":";
    const char* magnetOnKey = "\"magnet_on\":";

    const char* motorStepsKey = "\"motor_steps\":";
    const char* currentHeightKey = "\"current_height\":";

    const char* hammerTimeoutKey = "\"hammer_timeout\":";
    const char* interruptFlagKey = "\"interrupt_flag\":";
    const char* brakeTimeoutKey = "\"brake_timeout\":";
    
    char jsonOut[MQTT_PUB_BUFFER_SIZE];

public:
    /* Interrupt pin state ( set & cleared in code ) */
    bool eStop;
    bool doorOpen;
    bool fistLimit;
    bool anvilLimit;
    bool homeLimit;
    bool topLimit;
    bool pressure;

    /* Relay control state */
    bool brakeOn;
    bool magnetOn;
    
    int motorSteps;
    float currentHeight;

    bool hammerTimeout;
    bool interruptFlag;
    bool brakeTimeout;

    State() {
        eStop = false;
        doorOpen = false;
        fistLimit = false;
        anvilLimit = false;
        homeLimit = false;
        topLimit = false;
        pressure = false;
        
        brakeOn = false;
        magnetOn = false;

        motorSteps = 0;
        currentHeight = 0.0;

        hammerTimeout = false;
        interruptFlag = false;
        brakeTimeout = false;
    }

    /* MAKE READ ONLY AFTER DEBUG */
    // void parseFromJSON(const char* jsonString) {
    //     jsonParseBool(jsonString, eStopKey, eStop);
    //     jsonParseBool(jsonString, doorOpenKey, doorOpen);
    //     jsonParseBool(jsonString, fistLimitKey, fistLimit);
    //     jsonParseBool(jsonString, anvilLimitKey, anvilLimit);
    //     jsonParseBool(jsonString, homeLimitKey, homeLimit);
    //     jsonParseBool(jsonString, topLimitKey, topLimit);
    //     jsonParseBool(jsonString, pressureKey, pressure);
        
    //     jsonParseBool(jsonString, brakeOnKey, brakeOn);
    //     jsonParseBool(jsonString, magnetOnKey, magnetOn);

    //     jsonParseInt(jsonString, motorStepsKey, motorSteps);
    //     jsonParseFloat(jsonString, currentHeightKey, currentHeight);

    //     jsonParseBool(jsonString, hammerTimeoutKey, hammerTimeout);
    //     jsonParseBool(jsonString, interruptFlagKey, interruptFlag);
    //     jsonParseBool(jsonString, brakeTimeoutKey, brakeTimeout);
    // }
    
    char* serializeToJSON() {
        jsonSerializeStart(jsonOut);

        jsonSerializeBool(jsonOut, eStopKey, eStop);
        jsonSerializeBool(jsonOut, doorOpenKey, doorOpen);
        jsonSerializeBool(jsonOut, fistLimitKey, fistLimit);
        jsonSerializeBool(jsonOut, anvilLimitKey, anvilLimit);
        jsonSerializeBool(jsonOut, homeLimitKey, homeLimit);
        jsonSerializeBool(jsonOut, topLimitKey, topLimit);
        jsonSerializeBool(jsonOut, pressureKey, pressure);
        
        jsonSerializeBool(jsonOut, brakeOnKey, brakeOn);
        jsonSerializeBool(jsonOut, magnetOnKey, magnetOn); 

        jsonSerializeInt(jsonOut, motorStepsKey, motorSteps);
        jsonSerializeFloat(jsonOut, currentHeightKey, currentHeight);

        jsonSerializeBool(jsonOut, hammerTimeoutKey, hammerTimeout); 
        jsonSerializeBool(jsonOut, interruptFlagKey, interruptFlag); 
        jsonSerializeBool(jsonOut, brakeTimeoutKey, brakeTimeout); 

        jsonSerializeEnd(jsonOut);
        return jsonOut;
    }
    
    void debugPrintJSON() {
        Serial.printf("State.debugPrintJSON() :\n%s\n\n", jsonOut);
    }

};

#endif /* MODEL_STATE_H */