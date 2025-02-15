#include <Arduino.h>
#include "dc_secret.h" // not included in git repo; contains 'SECRET_XXXXXXX' values
#include "dc_esp_server.h"
#include "dc_alert.h"

#include "x_models.h"
#include "x_io.h"
#include "x_mqtt.h"
#include "x_machine.h"

#define SERIAL_BAUD 115200
#define SERIAL_DELAY 500

void setup() {

    Serial.begin(SERIAL_BAUD);
    while(!Serial) delay(SERIAL_DELAY);

    Serial.printf("\n\nSetting up device: %s...\n", SECRET_MQTT_DEVICE);

    setupFileSystem();

    setupWiFi(SECRET_WIFI_SSID, SECRET_WIFI_PW);

    setupMQTT_X(SECRET_MQTT_BROKER, SECRET_MQTT_PORT);

    setupIO();

}


void loop() {

    Alert* alert = nullptr;

    if( g_ops.diagnosticMode
    )   runDiagnosticMode();

    else 
        alert = runOperations();

    if( alert
    )   mqttPublishAlert(alert);             
 
    serviceMQTTClient_X(
        SECRET_MQTT_USER, 
        SECRET_MQTT_PW
    );

}
