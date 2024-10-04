#include "DisplayControl.h"
#include "ACControl.h"
#include "ConnectionControl.h"

TFT_eSPI tft = TFT_eSPI();

// Touchscreen pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

void initializeDisplay() {
  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);

  // Start the tft display
  tft.init();
  tft.setRotation(1);
  // Clear the screen before writing to it
  tft.fillScreen(TFT_BLACK);
}

void updateDisplay() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  
  // Display temperatures
  int yPos = 10;
  for (const auto& tank : tanks) {
    tft.setCursor(10, yPos);
    tft.printf("%s: %.1f C", tank.name.c_str(), tank.temperature);
    yPos += 30;
  }
  
  // Display WiFi and MQTT status
  tft.setCursor(10, yPos);
  tft.printf("WiFi: %s", connectedToWiFi ? "Connected" : "Disconnected");
  yPos += 20;
  tft.setCursor(10, yPos);
  tft.printf("MQTT: %s", mqttClient.connected() ? "Connected" : "Disconnected");
  
  // Display AC status
  yPos += 20;
  tft.setCursor(10, yPos);
  tft.printf("AC: %s", acRunning ? "ON" : "OFF");
  yPos += 20;
  tft.setCursor(10, yPos);
  tft.printf("Set Temp: %.1f C", acTurnOnTemp);
  
  drawButtons();
}

void drawButtons() {
  // Draw Connection Settings button
  tft.fillRect(10, SCREEN_HEIGHT - 60, SCREEN_WIDTH/2 - 20, 50, TFT_BLUE);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Connection", SCREEN_WIDTH/4, SCREEN_HEIGHT - 35);

  // Draw AC Settings button
  tft.fillRect(SCREEN_WIDTH/2 + 10, SCREEN_HEIGHT - 60, SCREEN_WIDTH/2 - 20, 50, TFT_RED);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("AC Settings", 3*SCREEN_WIDTH/4, SCREEN_HEIGHT - 35);
}

void checkTouchInput() {
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    int x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    int y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);

    Serial.printf("Touch detected at x=%d, y=%d\n", x, y);

    if (y > SCREEN_HEIGHT - 60) {
      if (x < SCREEN_WIDTH / 2) {
        Serial.println("Connection config area touched");
        handleConnectionConfig();
      } else {
        Serial.println("AC config area touched");
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
  tft.fillRect(10, SCREEN_HEIGHT - 40, 60, 30, TFT_BLUE);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Back", 40, SCREEN_HEIGHT - 25);

  while (inConfigMode && connectionConfigActive) {
    if (touchscreen.tirqTouched() && touchscreen.touched()) {
      TS_Point p = touchscreen.getPoint();
      int x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
      int y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
      
      if (y > SCREEN_HEIGHT - 40 && x < 70) {
        inConfigMode = false;
        connectionConfigActive = false;
        updateDisplay();
        break;
      }
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
  tft.fillRect(10, 100, 100, 30, TFT_BLUE);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(acRunning ? "Turn OFF" : "Turn ON", 60, 115);
  
  tft.fillRect(120, 100, 100, 30, TFT_GREEN);
  tft.drawString("Temp +", 170, 115);
  
  tft.fillRect(120, 140, 100, 30, TFT_RED);
  tft.drawString("Temp -", 170, 155);
  
  // Add a "Back" button
  tft.fillRect(10, SCREEN_HEIGHT - 40, 60, 30, TFT_BLUE);
  tft.drawString("Back", 40, SCREEN_HEIGHT - 25);

  while (true) {
    if (touchscreen.tirqTouched() && touchscreen.touched()) {
      TS_Point p = touchscreen.getPoint();
      int x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
      int y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
      
      if (y > SCREEN_HEIGHT - 40 && x < 70) {
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