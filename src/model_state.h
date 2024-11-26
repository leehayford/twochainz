#ifndef MODEL_STATE_H
#define MODEL_STATE_H

#include <Arduino.h>
#include <mutex>
#include "dc_esp_server.h"
#include "dc_json.h"

class State {
protected:
    std::mutex mut;

    const char* eStopKey = "\"estop\":";
    const char* doorOpenKey = "\"door_open\":";
    const char* fistLimitKey = "\"fist_limit\":";
    const char* anvilLimitKey = "\"anvil_limit\":";
    const char* topLimitKey = "\"top_limit\":";
    const char* pressureKey = "\"pressure\":";

    const char* brakeOnKey = "\"brake_on\":";
    const char* magnetOnKey = "\"magnet_on\":";
    const char* motorOnkey = "\"motor_on\":";

    const char* cyclesCompletedKey = "\"cycles_completed\":";
    const char* currentHeightKey = "\"current_height\":";
    const char* statusKey = "\"status\":";
    
    char jsonOut[MQTT_PUB_BUFFER_SIZE];

public:
    /* Interrupt pin state ( set & cleared in code ) */
    bool eStop;
    bool doorOpen;
    bool fistLimit;
    bool anvilLimit;
    bool topLimit;
    bool pressure;

    /* Relay control state */
    bool breakOn;
    bool magnetOn;
    bool motorOn;

    int cyclesCompleted;
    float currentHeight;
    char status[JSON_FIELD_STRING_LENGTH];

    // bool send;
// public:
    State(
        bool e_stop = false,        
        bool door_open = false,
        bool fist_limit = false,
        bool anvil_limit = false,
        bool top_limit = false,
        bool pressure = false,
        
        bool brake_on = false,
        bool magnet_on = false,
        bool motor_on = false,

        int cycles = 0,
        float height = 0.0,
        const char stat[JSON_FIELD_STRING_LENGTH]="initialized"
    ) {
        eStop = e_stop;
        doorOpen = door_open;
        fistLimit = fist_limit;
        anvilLimit = anvil_limit;
        topLimit = top_limit;
        pressure = pressure;
        
        breakOn = brake_on;
        magnetOn = magnet_on;
        motorOn = motor_on;

        cyclesCompleted = cycles;
        currentHeight = height;
        strcpy(status, stat);

        // send = false;
    }
    
    void setDoorClosed(bool b) {
        mut.lock();
        doorOpen = b;
        mut.unlock();
    }

    void setStatus(const char stat[JSON_FIELD_STRING_LENGTH]) {
        strcpy(status, stat);
    }
    
    /* TODO: MAKE READ ONLY AFTER DEBUG */
    void parseFromJSON(const char* jsonString) {
        jsonParseBool(jsonString, doorOpenKey, doorOpen);
        jsonParseBool(jsonString, fistLimitKey, fistLimit);
        jsonParseBool(jsonString, anvilLimitKey, anvilLimit);
        
        jsonParseBool(jsonString, brakeOnKey, breakOn);
        jsonParseBool(jsonString, magnetOnKey, magnetOn);

        jsonParseInt(jsonString, cyclesCompletedKey, cyclesCompleted);
        jsonParseFloat(jsonString, currentHeightKey, currentHeight);
        jsonParseString(jsonString, statusKey, status, sizeof(status));
    }
    
    char* serializeToJSON() {
        jsonSerializeStart(jsonOut);

        jsonSerializeBool(jsonOut, eStopKey, eStop);
        jsonSerializeBool(jsonOut, doorOpenKey, doorOpen);
        jsonSerializeBool(jsonOut, fistLimitKey, fistLimit);
        jsonSerializeBool(jsonOut, anvilLimitKey, anvilLimit);
        jsonSerializeBool(jsonOut, topLimitKey, topLimit);
        jsonSerializeBool(jsonOut, pressureKey, pressure);
        
        jsonSerializeBool(jsonOut, brakeOnKey, breakOn);
        jsonSerializeBool(jsonOut, magnetOnKey, magnetOn); 
        jsonSerializeBool(jsonOut, motorOnkey, motorOn); 

        jsonSerializeInt(jsonOut, cyclesCompletedKey, cyclesCompleted);
        jsonSerializeFloat(jsonOut, currentHeightKey, currentHeight);
        jsonSerializeString(jsonOut, statusKey, status);

        jsonSerializeEnd(jsonOut);
        return jsonOut;
    }
    
    void debugPrintJSON() {
        Serial.printf("State.debugPrintJSON() :\n%s\n\n", jsonOut);
    }
    
    void debugPrintValues() {
        Serial.printf("State.debugPrintValues() :\n");
        
        Serial.printf("Emergency stop: %s\n", btoa(eStop));
        Serial.printf("Door closed: %s\n", btoa(doorOpen));
        Serial.printf("Fist at hammer: %s\n", btoa(fistLimit));
        Serial.printf("Hammer at anvil_limit: %s\n", btoa(anvilLimit));
        Serial.printf("Fist at top_limit: %s\n", btoa(topLimit));
        Serial.printf("Brake pressure OK: %s\n", btoa(pressure));
        
        Serial.printf("Brake on: %s\n", btoa(breakOn));
        Serial.printf("Magnet on: %s\n", btoa(magnetOn));
        Serial.printf("Motor Enabled: %s\n", btoa(motorOn));

        Serial.printf("Cycles completed: %d\n", cyclesCompleted);
        Serial.printf("Current height: %f\n", currentHeight);
        Serial.printf("Status: %s\n\n", status);
    }

};

#endif /* MODEL_STATE_H */