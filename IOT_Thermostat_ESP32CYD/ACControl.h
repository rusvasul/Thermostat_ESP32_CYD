#ifndef AC_CONTROL_H
#define AC_CONTROL_H

#include <Arduino.h>

// AC Control Variables
extern float acTurnOnTemp;
extern float acTurnOffTemp;
extern bool acRunning;
extern bool fanRunning;
extern bool defrostHeaterRunning;
extern unsigned long acStartTime;
extern const unsigned long minRunTime;
extern const unsigned long defrostInterval;
extern unsigned long lastDefrostTime;
extern const unsigned long defrostDuration;
extern void startAC();
extern void stopAC();

// Function declarations
void initializeRelays();
void controlAC();
void startAC();
void stopAC();
void startFan();
void stopFan();
void startDefrostCycle();

#endif // AC_CONTROL_H