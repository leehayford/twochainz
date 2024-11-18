#include <Arduino.h>
#include "dc_secret.h" // not included in git repo; contains 'SECRET_XXXXXXX' values
#include "dc_esp_server.h"
#include "model_config.h"

#define SERIAL_BAUD 115200
#define SERIAL_DELAY 500

Config cfg;
void testConfig() {

    cfg.debugPrintValues();
    const char* jsonString = R"({
        "isloaded": true,
        "cycles": 30,
        "height": 5.5,
        "status": "Configuration set"
    })";
    
    cfg.parseJSONToConfig(jsonString);
    cfg.serializeConfigToJSON();
    cfg.debugPrintJSON();
  
    cfg.cycles = 27;
    cfg.height = 38.75;
    cfg.setStatus("Configuration altered");
    
    jsonString = cfg.serializeConfigToJSON();
    cfg.debugPrintJSON();
    
    cfg.parseJSONToConfig(jsonString);
    cfg.debugPrintValues();

}

void setup() {

    Serial.begin(SERIAL_BAUD);
    while(!Serial) delay(SERIAL_DELAY);

    setupFileSystem();

    setupWiFi(SECRET_WIFI_SSID, SECRET_WIFI_PW);

    // setupIO();

    // setupMQTTClient((mqttCallBackFunc)&mqttCallBack);

    // runWSServer((wsMsgHandleFunc)&wsMessageHandler);

    testConfig();

}

void loop() {
  // put your main code here, to run repeatedly:
}
