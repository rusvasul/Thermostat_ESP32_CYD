#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <TFT_eSPI.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <Preferences.h>

#include "Config.h"
#include "ACControl.h"
#include "DisplayControl.h"
#include "ConnectionControl.h"

// Objects
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
WiFiClient espClient;
PubSubClient mqttClient(espClient);
TFT_eSPI tft = TFT_eSPI();
Preferences preferences;
AsyncWebServer server(80);

// Global variables
std::vector<TankConfig> tanks;
bool connectedToWiFi = false;
String ssid, password, mqttServer, mqttUsername, mqttPassword;
int mqttPort = 1883;
unsigned long lastUpdateTime = 0;

// Configuration mode variables
bool inConfigMode = false;
bool connectionConfigActive = false;
bool acConfigActive = false;

void setup() {
  Serial.begin(115200);
  
   // Initialize Preferences
  preferences.begin("config", false);

  // Initialize TFT
  initializeDisplay();
  
  // Initialize DS18B20 sensors
  sensors.begin();
  
  // Initialize relay pins
  initializeRelays();
  
  // Auto-detect probes
  int deviceCount = sensors.getDeviceCount();
  for (int i = 0; i < deviceCount; i++) {
    TankConfig tank;
    tank.name = "Tank " + String(i + 1);
    tank.temperature = 0.0;
    tanks.push_back(tank);
  }
  
  esp_log_level_set("*", ESP_LOG_ERROR);
  loadSettings();
  WiFi.mode(WIFI_AP_STA);
  setupWiFi();
  setupMQTT();
  setupWebServer();

  // Initialize touch
  uint16_t calData[5] = { 295, 3493, 320, 3602, 2 };
  tft.setTouch(calData);
}

void loop() {
  unsigned long currentTime = millis();
  
  if (!inConfigMode) {
    if (currentTime - lastUpdateTime >= updateInterval) {
      lastUpdateTime = currentTime;
      
      updateTemperatures();
      controlAC();
      updateDisplay();
      
      if (connectedToWiFi) {
        handleMQTT();
      }
    }
    
    checkAndMaintainAP();
  } else {
    if (connectionConfigActive) {
      handleConnectionConfig();
    } else if (acConfigActive) {
      handleACConfig();
    }
  }
  
  checkTouchInput();
  delay(100);
}

void updateTemperatures() {
  sensors.requestTemperatures();
  for (size_t i = 0; i < tanks.size(); i++) {
    tanks[i].temperature = sensors.getTempCByIndex(i);
  }
}