#ifndef DC_ALERT_H
#define DC_ALERT_H

#include "dc_esp_server.h"
#include "dc_json.h"

typedef enum {
    SUCCES = 1, // see utils.js ALERT_CODES
    WARNING, 
    ERROR
} eAlertCode_t;

class Alert {
private: 
    /* FOR JSON */
    const char* headlineKey = "\"headline\":";
    const char* messageKey = "\"message\":";
    const char* codeKey = "\"code\":";
    char jsonOut[MQTT_PUB_BUFFER_SIZE];

    /* Data */
    char* headline;
    char* message;
    eAlertCode_t code;

public:
    Alert(const char* h, const char* m, eAlertCode_t c = ERROR) : 
        headline((char*)h),
        message((char*)m), 
        code(c) 
    {}
 
    char* getHeadline() { return headline; }

    char* getMessage() { return message; }

    eAlertCode_t getCode() { return code; }

    char* getJSON() {
        jsonSerializeStart(jsonOut);
        jsonSerializeString(jsonOut, headlineKey, headline);
        jsonSerializeString(jsonOut, messageKey, message);
        jsonSerializeInt(jsonOut, codeKey, (int)code);
        jsonSerializeEnd(jsonOut);
        return jsonOut;
    }
};

/* A bool and an Alert* */
typedef struct {bool bRes; Alert* err;} sboolAlert;

/* A int32 and an Alert* */
typedef struct {int32_t ui32Res; Alert* err;} si32Alert;

/* A uint32 and an Alert* */
typedef struct {uint32_t ui32Res; Alert* err;} sui32Alert;

/* A float and an Alert* */
typedef struct {float fRes; Alert* err;} sfAlert;

#endif /* DC_ALERT_H */