#include "dc_esp_server.h"


/* FILE SYSTEM */
void setupFileSystem() {
    if(!LittleFS.begin(true)){
        Serial.println("An error has occurred while mounting LittleFS");
        return;
    }
    File file = LittleFS.open(FS_TEST_FILE);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.println("FS Test File Content:");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
    Serial.println();
}


/* WIFI */
void setupWiFi(const char* ssid, const char* password) {
    Serial.print((String)"\nConnecting to " + ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.printf("\nConnected..! Got IP: %s\n", WiFi.localIP().toString());
}


/* WEBSERVER / WEBSOCKET */
AsyncWebServer webServer(WS_PORT);
AsyncWebSocket ws(WS_ROOT);
wsMsgHandleFunc wsMsgHandler;
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        (*wsMsgHandler)(data);
    }
}

void wsEventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void runWSServer(wsMsgHandleFunc func) {

    wsMsgHandler = func;

    ws.onEvent(wsEventHandler);
    webServer.addHandler(&ws);

    // Route for root / web page
    webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/index.html", "text/html");
    });

    // Serve web page from file system
    webServer.serveStatic("/", LittleFS, "/");

    // Start server
    webServer.begin();
}

void serviceClients() {
    ws.cleanupClients();
}

void sendWSString(String str) {
    ws.textAll(str);
}



/* MQTT */
WiFiClient mqttWifiClient;
PubSubClient mqttClient(mqttWifiClient);
void setupMQTTClient(const char* mqttBrokerIP, int mqttBrokerPort, mqttCallBackFunc func) {
    mqttClient.setServer(mqttBrokerIP, mqttBrokerPort);
    mqttClient.setCallback(func);
}

void serviceMQTTClient(mqttSubscription* subs, uint32_t length) {
    if (!mqttClient.connected()) {
        reconnectMqttClient(subs, length);
    }
    mqttClient.loop();
}

void reconnectMqttClient(mqttSubscription* subs, uint32_t length) {

    while (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");

        if (mqttClient.connect("DOIT-ESP32-DKV1-PIO-0", "nordicThingy", "run89ima9")) {
            Serial.println("MQTT conneted");
            for (uint32_t i = 0; i < length; i++) {
                mqttClient.subscribe(subs[i].topic.c_str());
                Serial.println("Subscribed to: " + subs[i].topic);
            }

        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);

        }
    }
}

void publishMQTTMessage(String topic, String msg) {
    
    uint32_t length = msg.length();
    byte* p = (byte*)malloc(length);
    memcpy(p, msg.c_str(), length);

    mqttClient.publish(topic.c_str(), p, length);
    free(p);
}

String mqttMessageToString(byte* msg, unsigned int length) {
    String str;
    for (int i = 0; i < length; i++) {
        str += (char)msg[i];
    }
    return str;
}

