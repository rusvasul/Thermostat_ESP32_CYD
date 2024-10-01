#include "ACControl.h"
#include "Config.h"

// AC Control Variables
float acTurnOnTemp = 25.0;
float acTurnOffTemp = 22.0;
bool acRunning = false;
bool fanRunning = false;
bool defrostHeaterRunning = false;
unsigned long acStartTime = 0;
const unsigned long minRunTime = 300000; // 5 minutes
const unsigned long defrostInterval = 3600000; // 1 hour
unsigned long lastDefrostTime = 0;
const unsigned long defrostDuration = 600000; // 10 minutes

void initializeRelays() {
    pinMode(COMPRESSOR_RELAY_PIN, OUTPUT);
    pinMode(FAN_RELAY_PIN, OUTPUT);
    pinMode(DEFROST_HEATER_PIN, OUTPUT);
    digitalWrite(COMPRESSOR_RELAY_PIN, LOW);
    digitalWrite(FAN_RELAY_PIN, LOW);
    digitalWrite(DEFROST_HEATER_PIN, LOW);
}

void controlAC() {
    float avgTemp = 0;
    for (const auto& tank : tanks) {
        avgTemp += tank.temperature;
    }
    avgTemp /= tanks.size();
    
    unsigned long currentTime = millis();
    bool shouldRunAC = avgTemp > acTurnOnTemp;
    bool shouldStopAC = avgTemp < acTurnOffTemp;
    
    if (currentTime - lastDefrostTime >= defrostInterval) {
        startDefrostCycle();
        return;
    }
    
    if (shouldRunAC && !acRunning) {
        startAC();
    } else if ((shouldStopAC && acRunning && currentTime - acStartTime >= minRunTime) || 
               (acRunning && currentTime - acStartTime >= 2 * minRunTime)) {
        stopAC();
    }
    
    if (acRunning && !fanRunning) {
        startFan();
    } else if (!acRunning && fanRunning) {
        stopFan();
    }
}

void startAC() {
    digitalWrite(COMPRESSOR_RELAY_PIN, HIGH);
    acRunning = true;
    acStartTime = millis();
}

void stopAC() {
    digitalWrite(COMPRESSOR_RELAY_PIN, LOW);
    acRunning = false;
}

void startFan() {
    digitalWrite(FAN_RELAY_PIN, HIGH);
    fanRunning = true;
}

void stopFan() {
    digitalWrite(FAN_RELAY_PIN, LOW);
    fanRunning = false;
}

void startDefrostCycle() {
    stopAC();
    stopFan();
    digitalWrite(DEFROST_HEATER_PIN, HIGH);
    defrostHeaterRunning = true;
    lastDefrostTime = millis();
    
    // This while loop might need to be adjusted depending on how you want to handle the defrost cycle
    while (millis() - lastDefrostTime < defrostDuration) {
        delay(1000);
        // You might want to call updateDisplay() here, but be careful about dependencies
    }
    
    digitalWrite(DEFROST_HEATER_PIN, LOW);
    defrostHeaterRunning = false;
}