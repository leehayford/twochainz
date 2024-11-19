#ifndef MODEL_STATE_H
#define MODEL_STATE_H

#include <Arduino.h>
#include <mutex>
#include "dc_json.h"

class State {
protected:
    std::mutex mut;

    const char* emergencyStopKey = "\"emergency_stop\":";
    const char* doorClosedKey = "\"door_closed\":";
    const char* fistContactKey = "\"fist_contact\":";
    const char* anvilContactKey = "\"anvil_contact\":";
    const char* topContactKey = "\"top_contact\":";
    const char* pressureContactKey = "\"pressure_contact\":";

    const char* brakeOnKey = "\"break_on\":";
    const char* magnetOnKey = "\"magnet_on\":";
    const char* motorEnabledkey = "\"motor_enabled\":";

    const char* cyclesCompletedKey = "\"cycles_completed\":";
    const char* currentHeightKey = "\"current_height\":";
    const char* statusKey = "\"status\":";
    
    char jsonOut[JSON_OBJECT_SERIALIZED_LENGTH];

public:
    /* Interrupt pin state ( set & cleared in code ) */
    bool emergencyStop;
    bool doorClosed;
    bool fistContact;
    bool anvilContact;
    bool topContact;
    bool pressureContact;

    /* Relay control state */
    bool breakOn;
    bool magnetOn;
    bool motorEnabled;

    int cyclesCompleted;
    float currentHeight;
    char status[JSON_FIELD_STRING_LENGTH];

    bool send;
// public:
    State(
        bool e_stop = false,        
        bool door = false,
        bool fist = false,
        bool anvil = false,
        bool top = false,
        bool pressure = false,
        
        bool brake = false,
        bool mag = false,
        bool mot_en = false,

        int cycles = 0,
        float height = 0.0,
        const char stat[JSON_FIELD_STRING_LENGTH]="initialized"
    ) {
        emergencyStop = e_stop;
        doorClosed = door;
        fistContact = fist;
        anvilContact = anvil;
        topContact = top;
        pressureContact = pressure;
        
        breakOn = brake;
        magnetOn = mag;
        motorEnabled = mot_en;

        cyclesCompleted = cycles;
        currentHeight = height;
        strcpy(status, stat);

        send = false;
    }
    
    void setDoorClosed(bool b) {
        mut.lock();
        doorClosed = b;
        mut.unlock();
    }

    void setStatus(const char stat[JSON_FIELD_STRING_LENGTH]) {
        strcpy(status, stat);
    }
    
    /* TODO: MAKE READ ONLY AFTER DEBUG */
    void parseJSONToState(const char* jsonString) {
        jsonParseBool(jsonString, doorClosedKey, doorClosed);
        jsonParseBool(jsonString, fistContactKey, fistContact);
        jsonParseBool(jsonString, anvilContactKey, anvilContact);
        
        jsonParseBool(jsonString, brakeOnKey, breakOn);
        jsonParseBool(jsonString, magnetOnKey, magnetOn);

        jsonParseInt(jsonString, cyclesCompletedKey, cyclesCompleted);
        jsonParseFloat(jsonString, currentHeightKey, currentHeight);
        jsonParseString(jsonString, statusKey, status, sizeof(status));
    }
    
    char* serializeStateToJSON() {
        jsonSerializeStart(jsonOut);

        jsonSerializeBool(jsonOut, emergencyStopKey, emergencyStop);
        jsonSerializeBool(jsonOut, doorClosedKey, doorClosed);
        jsonSerializeBool(jsonOut, fistContactKey, fistContact);
        jsonSerializeBool(jsonOut, anvilContactKey, anvilContact);
        jsonSerializeBool(jsonOut, topContactKey, topContact);
        jsonSerializeBool(jsonOut, pressureContactKey, pressureContact);
        
        jsonSerializeBool(jsonOut, brakeOnKey, breakOn);
        jsonSerializeBool(jsonOut, magnetOnKey, magnetOn); 
        jsonSerializeBool(jsonOut, motorEnabledkey, motorEnabled); 

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
        
        Serial.printf("Emergency stop: %s\n", btoa(emergencyStop));
        Serial.printf("Door closed: %s\n", btoa(doorClosed));
        Serial.printf("Fist at hammer: %s\n", btoa(fistContact));
        Serial.printf("Hammer at anvil: %s\n", btoa(anvilContact));
        Serial.printf("Fist at top: %s\n", btoa(topContact));
        Serial.printf("Brake pressure OK: %s\n", btoa(pressureContact));
        
        Serial.printf("Brake on: %s\n", btoa(breakOn));
        Serial.printf("Magnet on: %s\n", btoa(magnetOn));
        Serial.printf("Motor Enabled: %s\n", btoa(motorEnabled));

        Serial.printf("Cycles completed: %d\n", cyclesCompleted);
        Serial.printf("Current height: %f\n", currentHeight);
        Serial.printf("Status: %s\n\n", status);
    }

};

#endif /* MODEL_STATE_H */