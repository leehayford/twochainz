#include <Arduino.h>
#include "dc_secret.h" // not included in git repo; contains 'SECRET_XXXXXXX' values
#include "dc_esp_server.h"
#include "dc_error.h"

#include "x_models.h"
#include "x_io.h"
#include "x_mqtt.h"
#include "x_machine.h"

#define SERIAL_BAUD 115200
#define SERIAL_DELAY 500


Error ERR_TEST_ERR("this is a test of the Error class", SUCCES);

Error* errTestFunc(bool forceError) {
    if( forceError
    )   return &ERR_TEST_ERR;

    return nullptr;
}

bool errTestDone = false;
void runErrorTest() {
    
    if( !errTestDone
    ) {
        Error* err;

        err = errTestFunc(false);
        if(err) Serial.printf("\nERROR TEST: You should not be reading this...\n");
        else Serial.printf("\nERROR TEST: Null pointer error worked!\n");

        err = errTestFunc(true); 
        if(err                                      /* We got some kind of error */
        ) {
            Serial.printf("\nERROR TEST: Message Text: %s\n", err->getText());
            Serial.printf("\nERROR TEST: Message JSON: %s\n\n", err->getJSON());
            mqttPublishError(err);
        }
    
        errTestDone = true;
    }
}

void setup() {

    Serial.begin(SERIAL_BAUD);
    while(!Serial) delay(SERIAL_DELAY);

    setupFileSystem();

    setupWiFi(SECRET_WIFI_SSID, SECRET_WIFI_PW);

    // runWSServer((wsMsgHandleFunc)&wsMessageHandler);

    setupMQTT_X(SECRET_MQTT_BROKER, SECRET_MQTT_PORT);

    setupIO();

    checkStateIOPins();

    statusUpdate(STATUS_START);
}

Error* opsErr;
void loop() {

    serviceMQTTClient_X(SECRET_MQTT_USER, SECRET_MQTT_PW);
    
    /* TODO: REMOVE FOR PRODUCTION */ runErrorTest();

    opsErr = runOperations();
    if( opsErr
    )   mqttPublishError(opsErr);

}
