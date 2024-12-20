#ifndef DC_ESP_SERVER_H
#define DC_ESP_SERVER_H

#include <WiFi.h>
#include <AsyncTCP.h> // https://github.com/me-no-dev/AsyncTCP
#include <ESPAsyncWebServer.h> // https://github.com/me-no-dev/ESPAsyncWebServer
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/archive/master.zip
#include "LittleFS.h"


/* FILE SYSTEM */
#define FS_TEST_FILE "/fs_test.txt"
extern void setupFileSystem();


/* WIFI */
extern void setupWiFi(const char* ssid, const char* password);


/* WEBSERVER / WEBSOCKET */
#define WS_PORT 80
#define WS_ROOT "/ws"

typedef void (*wsMsgHandleFunc)(uint8_t*);
extern void runWSServer(wsMsgHandleFunc func);
extern void serviceClients();
extern void sendWSString(String str);


/* MQTT */
#define MQTT_PUB_BUFFER_SIZE 1024

typedef void (*mqttCMDFunc) (char* msg);
typedef struct {const char* topic; mqttCMDFunc func;} mqttSubscription;

typedef void (*mqttPubFunc) ();
typedef struct {const char* topic; int flag; mqttPubFunc func;} mqttPublication;

extern char* mqttTopic(const char* prfx, const char* topic);

typedef void (*mqttCallBackFunc) (char* topic, byte* message, unsigned int length);
extern void setupMQTTClient(const char* mqttBrokerIP, int mqttBrokerPort, mqttCallBackFunc func);

extern void serviceMQTTClient(const char* user, const char* pw, mqttSubscription* subs, int length);
extern void reconnectMqttClient(const char* user, const char* pw, mqttSubscription* subs, int length);
extern void publishMQTTMessage(const char* topic, char* msg);

#endif /* DC_ESP_SERVER_H */