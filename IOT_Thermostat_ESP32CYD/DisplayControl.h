#ifndef DISPLAY_CONTROL_H
#define DISPLAY_CONTROL_H

#include <TFT_eSPI.h>
#include <vector>
#include "Config.h"

extern TFT_eSPI tft;

// Function declarations
void initializeDisplay();
void updateDisplay();
void drawButtons();
void checkTouchInput();
void handleConnectionConfig();
void handleACConfig();

// External variables that need to be accessed
extern std::vector<TankConfig> tanks;
extern bool connectedToWiFi;
extern bool acRunning;
extern bool fanRunning;
extern bool defrostHeaterRunning;
extern float acTurnOnTemp;
extern String ssid;
extern String mqttServer;
extern bool inConfigMode;
extern bool connectionConfigActive;
extern bool acConfigActive;

#endif // DISPLAY_CONTROL_H