#ifndef MODEL_ADMIN_H
#define MODEL_ADMIN_H

#include <Arduino.h>
#include "dc_esp_server.h"
#include "dc_json.h"

#define ADMIN_MOT_REV_INCH 6.0
#define ADMIN_MOT_REV_STEP 2000
#define ADMIN_MOT_MAX_INCH 46.75
#define ADMIN_MOT_MAX_INCH_OVER 0.5
#define ADMIN_DEFAULT_FILE "/adm.js"

class Admin {
private:

    /* Operations */
    const char* opsTmrHammerKey = "\"ops_tmr_hammer_usec\":";
    const char* opsTmrBrakeKey = "\"ops_tmr_brake_usec\":";
    const char* opsPosPeriodKey = "\"ops_pos_period_msec\":";

    /* IO */
    const char* ioTmrITRDebKey = "\"io_tmr_itr_deb_usec\":";
    
    /* Motor */
    const char* motHzLowKey = "\"mot_hz_low\":";
    const char* motHzHighKey = "\"mot_hz_high\":";
    const char* motAccelKey = "\"mot_accel\":";
    const char* motDecelKey = "\"mot_decel\":";
    
    const char* motInchRevKey = "\"mot_inch_rev\":";
    const char* motStepsRevKey = "\"mot_steps_rev\":";

    const char* motInchMaxKey = "\"mot_inch_max\":";
    const char* motStepsMaxKey = "\"mot_steps_max\":";

    const char* motInchOverKey  = "\"mot_inch_over\":";
    const char* motStepsOverKey = "\"mot_steps_over\":";

    /* Diagnostic */
    const char* diagInchKey = "\"diag_inch\":";
    const char* diagStepsKey = "\"diag_steps\":";

    // const char* diagTestKey = "\"test_string\":"; /* TODO: REMOVE */
    // const char* diagTestKey = "\"test_int\":"; /* TODO: REMOVE */
    // const char* diagTestKey = "\"test_float\":"; /* TODO: REMOVE */
    // const char* diagTestKey = "\"test_bool\":"; /* TODO: REMOVE */
    
    char jsonOut[MQTT_PUB_BUFFER_SIZE];

public:

    /* Operations */
    int opsTmrHammer_uSec;
    int opsTmrBrake_uSec;
    int opsPosPeriod_mSec;

    /* IO */
    int ioTmrITRDeb_uSec;

    /* Motor */
    int motHzLow;
    int motHzHigh;
    int motAccel;
    int motDecel;

    float motInchRev;
    int motStepsRev;

    float motInchMax;
    int motStepsMax;

    float motInchOver;
    int motStepsOver;

    /* Diagnostic */
    float diagInch;
    int diagSteps;

    // char diagTest[JSON_FIELD_STRING_LENGTH]; /* TODO: REMOVE */
    // int diagTest; /* TODO: REMOVE */
    // float diagTest; /* TODO: REMOVE */
    // bool diagTest; /* TODO: REMOVE */

    Admin() {

        /* Operations */          
        opsTmrHammer_uSec = 1000000;         // 1.0 seconds for hammer to drop
        opsTmrBrake_uSec = 300000;          // 0.3 seconds to pressure to build / drop 
        opsPosPeriod_mSec = 300;            // 0.3 seconds between position updates

        /* IO */
        ioTmrITRDeb_uSec = 2000;            // 0.002 seconds default debounce 

        motHzLow = 500; 
        motHzHigh = 3500;
        motAccel = 4000;                    // Steps / sec / sec
        motDecel = 4000;                    // Steps / sec / sec

        motInchRev = ADMIN_MOT_REV_INCH;
        motStepsRev = ADMIN_MOT_REV_STEP;

        motInchMax = ADMIN_MOT_MAX_INCH;                // 46.75" height of top limit
        motStepsMax = getStepsFromInch(motInchMax);     // 46.75 / 6.0 * 2000 = 15583

        motInchOver = 0.099;                            // 0.099" distance to overshoot when returning home
        motStepsOver = getStepsFromInch(motInchOver);   // 0.099 / 6.0 * 2000 = 33              

        /* Diagnostic */
        diagInch = 0.002;                               // 0.002" default "jog" distance
        diagSteps = getStepsFromInch(diagInch);         // 0.002 / 6.0 * 2000 = 4

        // setTestString("some test string"); /* TODO: REMOVE */
        // diagTest = 999; /* TODO: REMOVE */
        // diagTest = 999.999; /* TODO: REMOVE */
        // diagTest = true; /* TODO: REMOVE */
    }

    /* TODO: REMOVE */
    // void setTestString(const char str[JSON_FIELD_STRING_LENGTH]) {
    //     strcpy(diagTest, str);
    // }

    /* called during parseFromJSON(...) */
    float validateMaxInch(float inch) {
        if( inch > ADMIN_MOT_MAX_INCH
        )   inch = ADMIN_MOT_MAX_INCH;

        if( inch < 1
        )   inch = 1;

        return inch;
    }

    /* called during parseFromJSON(...) */
    float validateInchOver(float inch) {
        if( inch > ADMIN_MOT_MAX_INCH_OVER
        )   inch = ADMIN_MOT_MAX_INCH_OVER;

        if( inch < ADMIN_MOT_MAX_INCH_OVER * -1
        )   inch = ADMIN_MOT_MAX_INCH_OVER * -1;

        return inch;
    }

    /* called during parseFromJSON(...) */
    float validateDiagInch(float inch) {
        if( inch > ADMIN_MOT_MAX_INCH
        )   inch = ADMIN_MOT_MAX_INCH;

        if( inch < ADMIN_MOT_MAX_INCH * -1
        )   inch = ADMIN_MOT_MAX_INCH * -1;

        return inch;
    }
    
    /* called during 
        - constructor...
        - parseFromJSON(...)  
        - getRecoverySteps() */
    int getStepsFromInch(float inch) { return inch / motInchRev * motStepsRev; }

    int getRecoverySteps() { return getStepsFromInch(motInchMax) * -1; }

    void parseFromJSON(const char* jsonString) {
        
        try {

            jsonParseInt(jsonString, opsTmrHammerKey, opsTmrHammer_uSec);
            jsonParseInt(jsonString, opsTmrBrakeKey, opsTmrBrake_uSec);
            jsonParseInt(jsonString, opsPosPeriodKey, opsPosPeriod_mSec);

            jsonParseInt(jsonString, ioTmrITRDebKey, ioTmrITRDeb_uSec);

            jsonParseInt(jsonString, motHzLowKey, motHzLow);
            jsonParseInt(jsonString, motHzHighKey, motHzHigh);
            jsonParseInt(jsonString, motAccelKey, motAccel);
            jsonParseInt(jsonString, motDecelKey, motDecel);

            jsonParseFloat(jsonString, motInchRevKey, motInchRev);
            jsonParseInt(jsonString, motStepsRevKey, motStepsRev);

            float inch = 0.0;
            jsonParseFloat(jsonString, motInchMaxKey, inch);
            motInchMax = validateMaxInch(inch);
            motStepsMax = getStepsFromInch(motInchMax);
            
            jsonParseFloat(jsonString, motInchOverKey, inch);
            motInchOver = validateInchOver(inch);
            motStepsOver = getStepsFromInch(motInchOver);
            
            jsonParseFloat(jsonString, diagInchKey, inch);
            diagInch = validateDiagInch(inch);
            diagSteps = getStepsFromInch(diagInch);
        
            // jsonParseString(jsonString, diagTestKey, diagTest, sizeof(diagTest)); /* TODO: REMOVE */
            // jsonParseInt(jsonString, diagTestKey, diagTest); /* TODO: REMOVE */
            // jsonParseFloat(jsonString, diagTestKey, diagTest); /* TODO: REMOVE */
            // jsonParseBool(jsonString, diagTestKey, diagTest); /* TODO: REMOVE */
            
        } catch (...) { 
            Serial.printf("\nmodel_admin.parseFromJSON : FAILED \n");    
            throw -1;
        }
        
        Serial.printf("\nmodel_admin.parseFromJSON : OK\n");
    }

    char* serializeToJSON() {
        jsonSerializeStart(jsonOut);

        jsonSerializeInt(jsonOut, opsTmrHammerKey, opsTmrHammer_uSec);
        jsonSerializeInt(jsonOut, opsTmrBrakeKey, opsTmrBrake_uSec);
        jsonSerializeInt(jsonOut, opsPosPeriodKey, opsPosPeriod_mSec);

        jsonSerializeInt(jsonOut, ioTmrITRDebKey, ioTmrITRDeb_uSec);

        jsonSerializeInt(jsonOut, motHzLowKey, motHzLow);
        jsonSerializeInt(jsonOut, motHzHighKey, motHzHigh);
        jsonSerializeInt(jsonOut, motAccelKey, motAccel);
        jsonSerializeInt(jsonOut, motDecelKey, motDecel);

        jsonSerializeFloat(jsonOut, motInchRevKey, motInchRev);
        jsonSerializeInt(jsonOut, motStepsRevKey, motStepsRev);

        jsonSerializeFloat(jsonOut, motInchMaxKey, motInchMax);
        jsonSerializeInt(jsonOut, motStepsMaxKey, motStepsMax);

        jsonSerializeFloat(jsonOut, motInchOverKey, motInchOver);
        jsonSerializeInt(jsonOut, motStepsOverKey, motStepsOver);

        jsonSerializeFloat(jsonOut, diagInchKey, diagInch);
        jsonSerializeInt(jsonOut, diagStepsKey, diagSteps);

        // jsonSerializeString(jsonOut, diagTestKey, diagTest); /* TODO: REMOVE */
        // jsonSerializeInt(jsonOut, diagTestKey, diagTest); /* TODO: REMOVE */
        // jsonSerializeFloat(jsonOut, diagTestKey, diagTest); /* TODO: REMOVE */
        // jsonSerializeBool(jsonOut, diagTestKey, diagTest); /* TODO: REMOVE */

        jsonSerializeEnd(jsonOut);
        return jsonOut;
    }
    
    void debugPrintJSON() {
        Serial.printf("Admin.debugPrintJSON() :\n%s\n\n", jsonOut);
    }
    
};


#endif /* MODEL_ADMIN_H */


