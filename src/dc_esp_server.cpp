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
    Serial.printf("\nConnected..! Got IP: %s\n\n", WiFi.localIP().toString());
}


/* WEBSERVER / WEBSOCKET */
AsyncWebServer m_webServer(WS_PORT);
AsyncWebSocket m_webSocket(WS_ROOT);
wsMsgHandleFunc m_wsMsgHandler;
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        (*m_wsMsgHandler)(data);
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

    m_wsMsgHandler = func;

    m_webSocket.onEvent(wsEventHandler);
    m_webServer.addHandler(&m_webSocket);

    // Route for root / web page
    m_webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/index.html", "text/html");
    });

    // Serve web page from file system
    m_webServer.serveStatic("/", LittleFS, "/");

    // Start server
    m_webServer.begin();
}

void serviceClients() {
    m_webSocket.cleanupClients();
}

void sendWSString(String str) {
    m_webSocket.textAll(str);
}



/* MQTT */
WiFiClient m_mqttWiFiClient;
PubSubClient m_mqttClient(m_mqttWiFiClient);

void setupMQTTClient(const char* mqttBrokerIP, int mqttBrokerPort, mqttCallBackFunc func) {
    m_mqttClient.setBufferSize(MQTT_PUB_BUFFER_SIZE);
    m_mqttClient.setServer(mqttBrokerIP, mqttBrokerPort);
    m_mqttClient.setCallback(func);
}

void serviceMQTTClient(const char* user, const char* pw, mqttSubscription* subs, int length) {
    if (!m_mqttClient.connected()) {
        reconnectMqttClient(user, pw, subs, length);
    }
    m_mqttClient.loop();
}

void reconnectMqttClient(const char* user, const char* pw, mqttSubscription* subs, int length) {

    while (!m_mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");

        if (m_mqttClient.connect("DOIT-ESP32-DKV1-PIO-1", user, pw)) {
            Serial.println("MQTT conneted");
            for (int i = 0; i < length; i++) {
                m_mqttClient.subscribe(subs[i].topic);
                Serial.printf("Subscribed to: %s\n", subs[i].topic);
            }

        } else {
            Serial.print("failed, rc=");
            Serial.print(m_mqttClient.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);

        }
    }
}

void publishMQTTMessage(char* topic, char* msg) {
    uint32_t length = strlen(msg);
    byte* p = (byte*)malloc(length);
    memcpy(p, msg, length);
    m_mqttClient.publish(topic, p, length);
    free(p);
}

