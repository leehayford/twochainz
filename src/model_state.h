#ifndef MODEL_STATE_H
#define MODEL_STATE_H

#include <Arduino.h>
#include "dc_json.h"

class State {
protected:
    const char* isDoorClosedKey = "\"is_door_closed\":";
    const char* isBrakeOnKey = "\"is_break_on\":";
    const char* isArmAtHammerKey = "\"is_arm_at_hammer\":";
    const char* isMagnetOnKey = "\"is_magnet_on\":";
    const char* isHammerAtAnvilKey = "\"is_hammer_at_anvil\":";
    const char* cyclesCompletedKey = "\"cycles_completed\":";
    const char* currentHeightKey = "\"current_height\":";
    const char* statusKey = "\"status\":";
    
    char jsonOut[JSON_OBJECT_SERIALIZED_LENGTH];

public:
    bool isDoorClosed;
    bool isBrakeOn;
    bool isArmAtHammer;
    bool isMagnetOn;
    bool isHammerAtAnvil;
    int cyclesCompleted;
    float currentHeight;
    char status[JSON_FIELD_STRING_LENGTH];

    State(
        bool door = false,
        bool brake = false,
        bool arm = false,
        bool mag = false,
        bool anvil = false,
        int cycles = 0,
        float height = 0.0,
        const char stat[JSON_FIELD_STRING_LENGTH]="initialized"
    ) {
        isDoorClosed = door;
        isBrakeOn = brake;
        isArmAtHammer = arm;
        isMagnetOn = mag;
        isHammerAtAnvil = anvil;
        cyclesCompleted = cycles;
        currentHeight = height;
        strcpy(status, stat);
    }
    
    void setStatus(const char stat[JSON_FIELD_STRING_LENGTH]) {
        strcpy(status, stat);
    }
    
    /* TODO: MAKE READ ONLY */
    void parseJSONToState(const char* jsonString) {
        jsonParseBool(jsonString, isDoorClosedKey, isDoorClosed);
        jsonParseBool(jsonString, isBrakeOnKey, isBrakeOn);
        jsonParseBool(jsonString, isArmAtHammerKey, isArmAtHammer);
        jsonParseBool(jsonString, isMagnetOnKey, isMagnetOn);
        jsonParseBool(jsonString, isHammerAtAnvilKey, isHammerAtAnvil);
        jsonParseInt(jsonString, cyclesCompletedKey, cyclesCompleted);
        jsonParseFloat(jsonString, currentHeightKey, currentHeight);
        jsonParseString(jsonString, statusKey, status, sizeof(status));
    }
    
    char* serializeStateToJSON() {
        jsonSerializeStart(jsonOut);
        jsonSerializeBool(jsonOut, isDoorClosedKey, isDoorClosed);
        jsonSerializeBool(jsonOut, isBrakeOnKey, isBrakeOn);
        jsonSerializeBool(jsonOut, isArmAtHammerKey, isArmAtHammer);
        jsonSerializeBool(jsonOut, isMagnetOnKey, isMagnetOn);
        jsonSerializeBool(jsonOut, isHammerAtAnvilKey, isHammerAtAnvil);
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
        Serial.printf("Is door closed: %s\n", btoa(isDoorClosed));
        Serial.printf("Is brake on: %s\n", btoa(isBrakeOn));
        Serial.printf("Is arm at hammer: %s\n", btoa(isArmAtHammer));
        Serial.printf("Is magnet on: %s\n", btoa(isMagnetOn));
        Serial.printf("Is hammer at anvil: %s\n", btoa(isHammerAtAnvil));
        Serial.printf("Cycles completed: %d\n", cyclesCompleted);
        Serial.printf("Current height: %f\n", currentHeight);
        Serial.printf("Status: %s\n\n", status);
    }

};

#endif /* MODEL_STATE_H */