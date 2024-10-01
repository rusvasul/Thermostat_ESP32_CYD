#include "Config.h"

// Pin Assignments
const int ONE_WIRE_BUS = 22;
const int TFT_LED_PIN = 21;
const int COMPRESSOR_RELAY_PIN = 32;
const int FAN_RELAY_PIN = 33;
const int DEFROST_HEATER_PIN = 25;

// WiFi and MQTT Settings
const char* DEFAULT_SSID = "ESP32_Config_AP";
const int MQTT_PORT = 1883;

// Timing Constants
const unsigned long UPDATE_INTERVAL = 10000; // 10 seconds
const unsigned long MIN_RUN_TIME = 300000; // 5 minutes
const unsigned long DEFROST_INTERVAL = 3600000; // 1 hour
const unsigned long DEFROST_DURATION = 600000; // 10 minutes
const unsigned long updateInterval = 10000; // 10 seconds

// Temperature Thresholds
const float DEFAULT_AC_TURN_ON_TEMP = 25.0;
const float DEFAULT_AC_TURN_OFF_TEMP = 22.0;

// Display Settings
const int DISPLAY_ROTATION = 1;