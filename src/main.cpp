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

    // runWSServer((wsMsgHandleFunc)&wsMessageHandler);

    setupMQTT_X(SECRET_MQTT_BROKER, SECRET_MQTT_PORT);

    setupIO();

    checkStateIOPins();
    statusUpdate(STATUS_START);
}

void loop() {

    serviceMQTTClient_X(SECRET_MQTT_USER, SECRET_MQTT_PW);

    runOperations();

}
