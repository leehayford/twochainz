#include "dc_esp_server.h"


/* FILE SYSTEM */
void setupFileSystem() {
    if(!LittleFS.begin(true)){
        Serial.println("An error has occurred while mounting LittleFS");
        return;
    }
    File file = LittleFS.open(FS_TEST_FILE, FILE_WRITE, FS_CREATE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    if(file.print("Test file write & read")){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    file.close();


    file = LittleFS.open(FS_TEST_FILE);
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

void writeToFile(const char* fileName, const char* text) {

    File file = LittleFS.open(fileName, FILE_WRITE, FS_CREATE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    
    if(file.print(text)){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    file.close();
}

void readFromFile(const char* fileName) {
    File file = LittleFS.open(fileName);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.printf("\n%s Content:\n", fileName);
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

/* WIFI */
void setupWiFi(const char* ssid, const char* password) {
    Serial.printf("\nConnecting to %s", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(2000);
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
void mqttCMDBuilder(char* fullTopic, const char* topic) {
    mqttTopicBuilder(fullTopic, "/cmd/", topic);
}
void mqttSIGBuilder(char* fullTopic, const char* topic) {
    mqttTopicBuilder(fullTopic, "/sig/", topic);
}
void mqttTopicBuilder(char* fullTopic, const char* prfx, const char* topic) {
    strcat((char*) fullTopic, prfx);
    strcat((char*) fullTopic, topic);
}

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

        if (m_mqttClient.connect(SECRET_MQTT_CLIENT, user, pw)) {
            Serial.println("MQTT conneted");
            
            char fullTopic[50] = SECRET_MQTT_DEVICE;
            for (int i = 0; i < length; i++) {
                strcpy(fullTopic, SECRET_MQTT_DEVICE);
                mqttCMDBuilder(fullTopic, subs[i].topic);
                m_mqttClient.subscribe(fullTopic);
                Serial.printf("Subscribed to: %s\n", fullTopic);
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

void publishMQTTMessage(const char* topic, char* msg) {
    uint32_t length = strlen(msg);
    byte* p = (byte*)malloc(length);
    memcpy(p, msg, length);
    
    // Serial.printf("Publishing to: %s\n", topic);
    m_mqttClient.publish(topic, p, length);
    free(p);
}

