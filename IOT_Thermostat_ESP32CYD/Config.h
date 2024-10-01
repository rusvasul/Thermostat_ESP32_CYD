#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <Arduino.h>

struct TankConfig {
    String name;
    float temperature;
};

extern std::vector<TankConfig> tanks;
extern const unsigned long updateInterval;

// Pin Assignments
extern const int ONE_WIRE_BUS;
extern const int TFT_LED_PIN;
extern const int COMPRESSOR_RELAY_PIN;
extern const int FAN_RELAY_PIN;
extern const int DEFROST_HEATER_PIN;

// WiFi and MQTT Settings
extern const char* DEFAULT_SSID;
extern const int MQTT_PORT;

// Timing Constants
extern const unsigned long UPDATE_INTERVAL;
extern const unsigned long MIN_RUN_TIME;
extern const unsigned long DEFROST_INTERVAL;
extern const unsigned long DEFROST_DURATION;

// Temperature Thresholds
extern const float DEFAULT_AC_TURN_ON_TEMP;
extern const float DEFAULT_AC_TURN_OFF_TEMP;

// Display Settings
extern const int DISPLAY_ROTATION;

#endif // CONFIG_H