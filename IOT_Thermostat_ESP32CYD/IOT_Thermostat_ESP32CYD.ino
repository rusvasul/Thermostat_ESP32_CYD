#include <OneWire.h>              // Author: Paul Stoffregen, et al. | Version: 2.3.8 (2024)
#include <DallasTemperature.h>    // Author: Miles Burton | Version: 3.9.0 (2020)
#include <WiFi.h>                 // Author: Arduino | Version: 2.0.0 (2022) - Part of ESP32 Arduino Core
#include <PubSubClient.h>         // Author: Nick O'Leary | Version: 2.8.0 (2021)
#include <TFT_eSPI.h>             // Author: Bodmer | Version: 2.5.0 (2023)
#include <Preferences.h>          // Author: Espressif Systems | Version: 2.0.0 (2022) - Part of ESP32 Arduino Core
#include <ESPAsyncWebServer.h>    // Author: Me No Dev | Version: 1.2.3 (2019)
#include <AsyncTCP.h>             // Author: Me No Dev | Version: 1.1.1 (2019)
#include <ArduinoJson.h>          // Author: Benoit Blanchon | Version: 6.21.2 (2023)

#include "Config.h"
#include "ACControl.h"
#include "DisplayControl.h"
#include "ConnectionControl.h"

// Objects
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
WiFiClient espClient;
PubSubClient mqttClient(espClient);
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
      checkTouchInput();
      
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