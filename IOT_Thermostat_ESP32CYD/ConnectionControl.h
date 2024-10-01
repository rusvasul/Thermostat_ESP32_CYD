#ifndef CONNECTION_CONTROL_H
#define CONNECTION_CONTROL_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <Preferences.h>
#include "Config.h"

extern WiFiClient espClient;
extern PubSubClient mqttClient;
extern AsyncWebServer server;
extern Preferences preferences;

extern bool connectedToWiFi;
extern String ssid, password, mqttServer, mqttUsername, mqttPassword;
extern int mqttPort;

void setupWiFi();
void checkAndMaintainAP();
void reconnectMQTT();
void publishTemperatures();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void setupWebServer();
void saveSettings();
void loadSettings();
void setupMQTT();
void handleMQTT();
String generateConfigPage();

#endif // CONNECTION_CONTROL_H