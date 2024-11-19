#include <Arduino.h>
#include "dc_secret.h" // not included in git repo; contains 'SECRET_XXXXXXX' values
#include "dc_esp_server.h"

#include "x_models.h"
#include "x_io.h"

#define SERIAL_BAUD 115200
#define SERIAL_DELAY 500

/* TODO: REMOVE AFTER DEBUG */
void testConfig() {
    cfg.debugPrintValues();
    cfg.cycles = 27;
    cfg.height = 38.75;
    
    const char* json = cfg.serializeConfigToJSON();
    cfg.debugPrintJSON();
    
    cfg.parseJSONToConfig(json);
    cfg.debugPrintValues();
}

/* TODO: REMOVE AFTER DEBUG */
void testState() {
    sta.debugPrintValues();
    sta.doorClosed = true;
    sta.breakOn = false;
    sta.fistContact = true;
    sta.magnetOn = true;
    sta.cyclesCompleted = 1;
    sta.currentHeight = 23.5;
    sta.setStatus("State altered");

    const char* json = sta.serializeStateToJSON();
    sta.debugPrintJSON();

    sta.parseJSONToState(json);
    sta.debugPrintValues();
}

long update = 3000;
unsigned long prev = 0;
unsigned long now = 0;

void setup() {

    Serial.begin(SERIAL_BAUD);
    while(!Serial) delay(SERIAL_DELAY);

    setupFileSystem();

    setupWiFi(SECRET_WIFI_SSID, SECRET_WIFI_PW);

    setupIO();

    // setupMQTTClient((mqttCallBackFunc)&mqttCallBack);

    // runWSServer((wsMsgHandleFunc)&wsMessageHandler);

    
    /* TODO: REMOVE AFTER DEBUG */
    // testConfig();
    // testState();
    motorEnable();
    /* TODO: REMOVE AFTER DEBUG */

    prev = millis() + update;
}

void loop() {
    // now = millis();
    // if (now > prev + update) {
    //     prev = now;
    if (sta.send == true) {
        sta.send = false;
        sta.debugPrintValues();
    }
    
    /* TODO: REMOVE AFTER DEBUG */
    motorBackNForth();
    /* TODO: REMOVE AFTER DEBUG */
}
