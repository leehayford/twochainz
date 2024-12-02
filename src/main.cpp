#include <Arduino.h>
#include "dc_secret.h" // not included in git repo; contains 'SECRET_XXXXXXX' values
#include "dc_esp_server.h"

#include "x_models.h"
#include "x_io.h"
#include "x_mqtt.h"
#include "x_machine.h"

#define SERIAL_BAUD 115200
#define SERIAL_DELAY 500

void setup() {

    Serial.begin(SERIAL_BAUD);
    while(!Serial) delay(SERIAL_DELAY);

    setupFileSystem();

    setupWiFi(SECRET_WIFI_SSID, SECRET_WIFI_PW);

    setupMQTT_X(SECRET_MQTT_BROKER, SECRET_MQTT_PORT);

    // runWSServer((wsMsgHandleFunc)&wsMessageHandler);

    setupIO();

    setupOps();
    
    checkStateIOPins();
    g_ops.setStatus(STATUS_START);
    setMQTTPubFlag(PUB_STATE);
    setMQTTPubFlag(PUB_CONFIG);
    setMQTTPubFlag(PUB_OPS);
}

void loop() {

    serviceMQTTClient_X(SECRET_MQTT_USER, SECRET_MQTT_PW);

    runOperations();

    /* TODO: REMOVE AFTER DEBUG */
    // motorBackNForth();
    /* TODO: REMOVE AFTER DEBUG */
}
