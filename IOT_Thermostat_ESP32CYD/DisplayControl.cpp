#include "DisplayControl.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "Config.h"
#include "ACControl.h"

extern TFT_eSPI tft;
extern PubSubClient mqttClient;

void initializeDisplay() {
    tft.init();
    tft.setRotation(DISPLAY_ROTATION);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);

    // Initialize touch
    uint16_t calData[5] = { 295, 3493, 320, 3602, 2 };
    tft.setTouch(calData);
}

void updateDisplay() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);

    // Calculate cell dimensions
    int cellWidth = tft.width();
    int topCellHeight = tft.height() * 2 / 3;
    int bottomCellHeight = tft.height() - topCellHeight;

    // Draw cell borders
    tft.drawRect(0, 0, cellWidth, topCellHeight, TFT_GREEN);
    tft.drawRect(0, topCellHeight, cellWidth / 2, bottomCellHeight, TFT_GREEN);
    tft.drawRect(cellWidth / 2, topCellHeight, cellWidth / 2, bottomCellHeight, TFT_GREEN);

    // Top cell: Temperature readings
    tft.setTextSize(2);
    int yPos = 10;
    for (const auto& tank : tanks) {
        tft.setCursor(10, yPos);
        tft.printf("%s: %.1f C", tank.name.c_str(), tank.temperature);
        yPos += 30;
    }

    drawButtons();
}

void drawButtons() {
    int cellWidth = tft.width();
    int topCellHeight = tft.height() * 2 / 3;
    int bottomCellHeight = tft.height() - topCellHeight;

    tft.setTextSize(1);
    
    // Connection button
    tft.setCursor(5, topCellHeight + 5);
    tft.println("Connection Settings");
    tft.setCursor(5, topCellHeight + 20);
    tft.printf("WiFi: %s", connectedToWiFi ? "Connected" : "Disconnected");
    tft.setCursor(5, topCellHeight + 35);
    tft.printf("MQTT: %s", mqttClient.connected() ? "Connected" : "Disconnected");
    // Add webhost connection information
    tft.setCursor(5, topCellHeight + 50);
      if (connectedToWiFi) {
          tft.printf("Setup: http://%s", WiFi.localIP().toString().c_str());
      } else {
          tft.printf("Setup: Connect to %s", DEFAULT_SSID);
      
      }

    // AC System button
    tft.setCursor(cellWidth / 2 + 5, topCellHeight + 5);
    tft.println("AC System Settings");
    tft.setCursor(cellWidth / 2 + 5, topCellHeight + 20);
    tft.printf("AC: %s", acRunning ? "ON" : "OFF");
    tft.setCursor(cellWidth / 2 + 5, topCellHeight + 35);
    tft.printf("Set Temp: %.1f C", acTurnOnTemp);
}

void checkTouchInput() {
    uint16_t x, y;
    if (tft.getTouch(&x, &y)) {
        int cellWidth = tft.width();
        int topCellHeight = tft.height() * 2 / 3;
        
        if (y > topCellHeight) {
            if (x < cellWidth / 2) {
                handleConnectionConfig();
            } else {
                handleACConfig();
            }
        }
    }
}

void handleConnectionConfig() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("Connection Settings");
    
    // Display current WiFi and MQTT settings
    tft.setTextSize(1);
    tft.setCursor(10, 50);
    tft.printf("WiFi SSID: %s", ssid.c_str());
    tft.setCursor(10, 70);
    tft.printf("MQTT Server: %s", mqttServer.c_str());
    
    // Add a "Back" button
    tft.drawRect(10, tft.height() - 40, 60, 30, TFT_GREEN);
    tft.setCursor(25, tft.height() - 30);
    tft.print("Back");
    
    // Handle touch input for this screen
    uint16_t x, y;
    while (inConfigMode && connectionConfigActive) {
        if (tft.getTouch(&x, &y)) {
            if (y > tft.height() - 40 && x < 70) {
                inConfigMode = false;
                connectionConfigActive = false;
                updateDisplay();
                break;
            }
            // Handle other button presses for settings
        }
        delay(100);
    }
}

void handleACConfig() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("AC System Settings");
    
    // Display current AC settings
    tft.setTextSize(1);
    tft.setCursor(10, 50);
    tft.printf("AC Status: %s", acRunning ? "ON" : "OFF");
    tft.setCursor(10, 70);
    tft.printf("Set Temperature: %.1f C", acTurnOnTemp);
    
    // Add control buttons
    tft.drawRect(10, 100, 100, 30, TFT_GREEN);
    tft.setCursor(20, 110);
    tft.print(acRunning ? "Turn OFF" : "Turn ON");
    
    tft.drawRect(120, 100, 100, 30, TFT_GREEN);
    tft.setCursor(130, 110);
    tft.print("Temp +");
    
    tft.drawRect(120, 140, 100, 30, TFT_GREEN);
    tft.setCursor(130, 150);
    tft.print("Temp -");
    
    // Add a "Back" button
    tft.drawRect(10, tft.height() - 40, 60, 30, TFT_GREEN);
    tft.setCursor(25, tft.height() - 30);
    tft.print("Back");
    
    // Handle touch input for this screen
    uint16_t x, y;
    while (true) {
        if (tft.getTouch(&x, &y)) {
            if (y > tft.height() - 40 && x < 70) {
                updateDisplay();
                break;
            } else if (y > 100 && y < 130) {
                if (x > 10 && x < 110) {
                    acRunning = !acRunning;
                    if (acRunning) startAC(); else stopAC();
                } else if (x > 120 && x < 220) {
                    acTurnOnTemp += 0.5;
                    acTurnOffTemp += 0.5;
                }
            } else if (y > 140 && y < 170 && x > 120 && x < 220) {
                acTurnOnTemp -= 0.5;
                acTurnOffTemp -= 0.5;
            }
            // Redraw the screen to show updated values
            handleACConfig();
        }
        delay(100);
    }
}