#ifndef DC_ERROR_H
#define DC_ERROR_H

#include "dc_esp_server.h"
#include "dc_json.h"

typedef enum {
    SUCCES = 1, // see utils.js ALERT_CODES
    WARNING, 
    ERROR
} eErrCode_t;

class Error {
private: 
    /* FOR JSON */
    const char* messageKey = "\"message\":";
    const char* codeKey = "\"code\":";
    char jsonOut[MQTT_PUB_BUFFER_SIZE];

    /* Data */
    char* message;
    eErrCode_t code;

public:
    Error(const char* m, eErrCode_t c = ERROR) : 
        message((char*)m), 
        code(c) 
    {}

    char* getText() { return message; }

    char* getJSON() {
        jsonSerializeStart(jsonOut);
        jsonSerializeString(jsonOut, messageKey, message);
        jsonSerializeInt(jsonOut, codeKey, (int)code);
        jsonSerializeEnd(jsonOut);
        return jsonOut;
    }
};

/* A bool and an Error* */
typedef struct {bool bRes; Error* err;} sboolErr;

/* A int32 and an Error* */
typedef struct {int32_t ui32Res; Error* err;} si32Err;

/* A uint32 and an Error* */
typedef struct {uint32_t ui32Res; Error* err;} sui32Err;

/* A float and an Error* */
typedef struct {float fRes; Error* err;} sfErr;

#endif /* DC_ERROR_H */