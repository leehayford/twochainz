#include <Arduino.h>
#include "dc_secret.h" // not included in git repo; contains 'SECRET_XXXXXXX' values
#include "dc_esp_server.h"

#include "x_models.h"
#include "x_io.h"
#include "x_mqtt.h"

#define SERIAL_BAUD 115200
#define SERIAL_DELAY 500

void setup() {

    Serial.begin(SERIAL_BAUD);
    while(!Serial) delay(SERIAL_DELAY);

    setupFileSystem();

    setupWiFi(SECRET_WIFI_SSID, SECRET_WIFI_PW);

    setupIO();

    setupXMQTT(SECRET_MQTT_BROKER, SECRET_MQTT_PORT);

    // runWSServer((wsMsgHandleFunc)&wsMessageHandler);

}

bool up = false;
void loop() {

    serviceMQTTClient(SECRET_MQTT_USER, SECRET_MQTT_PW, subs, N_SUBS);

    if (sta.send == true) {
        sta.debugPrintValues();
        sta.send = false;
    }
    
    if (!up) {
        publishMQTTMessage((char*)"esp32/sig/start", (char *)"Move Bitch! Get out the way!");
        up = true;
    }

    /* TODO: REMOVE AFTER DEBUG */
    motorBackNForth();
    /* TODO: REMOVE AFTER DEBUG */
}
