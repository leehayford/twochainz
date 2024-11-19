#ifndef MODEL_STATE_H
#define MODEL_STATE_H

#include <Arduino.h>
#include "dc_json.h"

class State {
protected:
    const char* doorClosedKey = "\"door_closed\":";
    const char* armContactKey = "\"arm_contact\":";
    const char* anvilContactKey = "\"anvil_contact\":";

    const char* brakeOnKey = "\"break_on\":";
    const char* magnetOnKey = "\"magnet_on\":";

    const char* cyclesCompletedKey = "\"cycles_completed\":";
    const char* currentHeightKey = "\"current_height\":";
    const char* statusKey = "\"status\":";
    
    char jsonOut[JSON_OBJECT_SERIALIZED_LENGTH];

public:
    /* Interrupt pin state ( set & cleared in code ) */
    bool doorClosed;
    bool armContact;
    bool anvilContact;

    /* Relay control state */
    bool breakOn;
    bool magnetOn;

    int cyclesCompleted;
    float currentHeight;
    char status[JSON_FIELD_STRING_LENGTH];

    State(
        bool door = false,
        bool arm = false,
        bool anvil = false,
        
        bool brake = false,
        bool mag = false,

        int cycles = 0,
        float height = 0.0,
        const char stat[JSON_FIELD_STRING_LENGTH]="initialized"
    ) {
        doorClosed = door;
        armContact = arm;
        anvilContact = anvil;
        
        breakOn = brake;
        magnetOn = mag;

        cyclesCompleted = cycles;
        currentHeight = height;
        strcpy(status, stat);
    }
    
    void setStatus(const char stat[JSON_FIELD_STRING_LENGTH]) {
        strcpy(status, stat);
    }
    
    /* TODO: MAKE READ ONLY AFTER DEBUG */
    void parseJSONToState(const char* jsonString) {
        jsonParseBool(jsonString, doorClosedKey, doorClosed);
        jsonParseBool(jsonString, armContactKey, armContact);
        jsonParseBool(jsonString, anvilContactKey, anvilContact);
        
        jsonParseBool(jsonString, brakeOnKey, breakOn);
        jsonParseBool(jsonString, magnetOnKey, magnetOn);

        jsonParseInt(jsonString, cyclesCompletedKey, cyclesCompleted);
        jsonParseFloat(jsonString, currentHeightKey, currentHeight);
        jsonParseString(jsonString, statusKey, status, sizeof(status));
    }
    
    char* serializeStateToJSON() {
        jsonSerializeStart(jsonOut);

        jsonSerializeBool(jsonOut, doorClosedKey, doorClosed);
        jsonSerializeBool(jsonOut, armContactKey, armContact);
        jsonSerializeBool(jsonOut, anvilContactKey, anvilContact);
        
        jsonSerializeBool(jsonOut, brakeOnKey, breakOn);
        jsonSerializeBool(jsonOut, magnetOnKey, magnetOn); 

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
        
        Serial.printf("Is door closed: %s\n", btoa(doorClosed));
        Serial.printf("Is arm at hammer: %s\n", btoa(armContact));
        Serial.printf("Is hammer at anvil: %s\n", btoa(anvilContact));
        
        Serial.printf("Is brake on: %s\n", btoa(breakOn));
        Serial.printf("Is magnet on: %s\n", btoa(magnetOn));

        Serial.printf("Cycles completed: %d\n", cyclesCompleted);
        Serial.printf("Current height: %f\n", currentHeight);
        Serial.printf("Status: %s\n\n", status);
    }

};

#endif /* MODEL_STATE_H */