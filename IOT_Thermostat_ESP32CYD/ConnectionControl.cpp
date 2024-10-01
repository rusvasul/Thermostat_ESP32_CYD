#include "ConnectionControl.h"
#include "ACControl.h"
#include "DisplayControl.h"
#include "Config.h"

void setupWiFi() {
    WiFi.softAP(DEFAULT_SSID);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    if (ssid.length() > 0) {
        WiFi.begin(ssid.c_str(), password.c_str());
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        if (WiFi.status() == WL_CONNECTED) {
            connectedToWiFi = true;
            Serial.println("\nConnected to WiFi");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("\nFailed to connect to WiFi");
        }
    }
    
    updateDisplay();
}

void checkAndMaintainAP() {
    static unsigned long lastCheck = 0;
    const unsigned long checkInterval = 30000;
    if (millis() - lastCheck >= checkInterval) {
        lastCheck = millis();
        if (WiFi.softAPgetStationNum() == 0) {
            WiFi.softAPdisconnect(true);
            delay(100);
            WiFi.softAP(DEFAULT_SSID);
            Serial.println("AP restarted");
        }
    }
}

void handleMQTT() {
    if (!mqttClient.connected()) {
        reconnectMQTT();
    }
    mqttClient.loop();
    publishTemperatures();
}

void setupMQTT() {
    mqttClient.setServer(mqttServer.c_str(), mqttPort);
    mqttClient.setCallback(mqttCallback);
}

void reconnectMQTT() {
    while (!mqttClient.connected()) {
        if (mqttClient.connect("ESP32Client", mqttUsername.c_str(), mqttPassword.c_str())) {
            mqttClient.subscribe("esp32/ac/#");
        } else {
            delay(5000);
        }
    }
}

void publishTemperatures() {
    for (const auto& tank : tanks) {
        String topic = "esp32/tank/" + tank.name + "/temperature";
        String payload = String(tank.temperature, 2);
        mqttClient.publish(topic.c_str(), payload.c_str());
    }
    mqttClient.publish("esp32/ac/status", acRunning ? "ON" : "OFF");
    mqttClient.publish("esp32/fan/status", fanRunning ? "ON" : "OFF");
    mqttClient.publish("esp32/defrost/status", defrostHeaterRunning ? "ON" : "OFF");
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    if (String(topic) == "esp32/ac/set") {
        if (message == "ON") startAC();
        else if (message == "OFF") stopAC();
    } else if (String(topic) == "esp32/fan/set") {
        if (message == "ON") startFan();
        else if (message == "OFF") stopFan();
    } else if (String(topic) == "esp32/ac/settemp") {
        float newTemp = message.toFloat();
        if (newTemp > 0 && newTemp < 40) {
            acTurnOnTemp = newTemp;
            acTurnOffTemp = newTemp - 3;
            saveSettings();
        }
    }
}

void setupWebServer() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = generateConfigPage();
        request->send(200, "text/html", html);
    });
    
    server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("ssid", true)) {
            ssid = request->getParam("ssid", true)->value();
            password = request->getParam("password", true)->value();
            mqttServer = request->getParam("mqtt_server", true)->value();
            mqttPort = request->getParam("mqtt_port", true)->value().toInt();
            mqttUsername = request->getParam("mqtt_username", true)->value();
            mqttPassword = request->getParam("mqtt_password", true)->value();
            
            for (size_t i = 0; i < tanks.size(); i++) {
                tanks[i].name = request->getParam("tank_name_" + String(i), true)->value();
            }
            
            saveSettings();
            request->send(200, "text/plain", "Settings saved. Restarting...");
            delay(1000);
            ESP.restart();
        } else {
            request->send(400, "text/plain", "Invalid request");
        }
    });
    
    server.begin();
}

String generateConfigPage() {
    String html = "<html><head><title>ESP32 Config</title></head><body>";
    html += "<h1>ESP32 Configuration</h1>";
    html += "<form action='/save' method='POST'>";
    html += "WiFi SSID: <input type='text' name='ssid' value='" + ssid + "'><br>";
    html += "WiFi Password: <input type='text' name='password' value='" + password + "'><br>";
    html += "MQTT Server: <input type='text' name='mqtt_server' value='" + mqttServer + "'><br>";
    html += "MQTT Port: <input type='number' name='mqtt_port' value='" + String(mqttPort) + "'><br>";
    html += "MQTT Username: <input type='text' name='mqtt_username' value='" + mqttUsername + "'><br>";
    html += "MQTT Password: <input type='text' name='mqtt_password' value='" + mqttPassword + "'><br>";
    
    for (size_t i = 0; i < tanks.size(); i++) {
        html += "Tank " + String(i + 1) + " Name: <input type='text' name='tank_name_" + String(i) + "' value='" + tanks[i].name + "'><br>";
    }
    
    html += "<input type='submit' value='Save'>";
    html += "</form></body></html>";
    return html;
}

void loadSettings() {
   // preferences.begin("config", true);
    ssid = preferences.getString("ssid", "");
    password = preferences.getString("password", "");
    mqttServer = preferences.getString("mqtt_server", "broker.hivemq.com");
    mqttPort = preferences.getInt("mqtt_port", 1883);
    mqttUsername = preferences.getString("mqtt_username", "");
    mqttPassword = preferences.getString("mqtt_password", "");
    acTurnOnTemp = preferences.getFloat("acTurnOnTemp", 25.0);
    acTurnOffTemp = preferences.getFloat("acTurnOffTemp", 22.0);
    for (size_t i = 0; i < tanks.size(); i++) {
        String key = "tank_name_" + String(i);
        tanks[i].name = preferences.getString(key.c_str(), tanks[i].name);
    }
   // preferences.end();
}

void saveSettings() {
   // preferences.begin("config", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.putString("mqtt_server", mqttServer);
    preferences.putInt("mqtt_port", mqttPort);
    preferences.putString("mqtt_username", mqttUsername);
    preferences.putString("mqtt_password", mqttPassword);
    preferences.putFloat("acTurnOnTemp", acTurnOnTemp);
    preferences.putFloat("acTurnOffTemp", acTurnOffTemp);
    for (size_t i = 0; i < tanks.size(); i++) {
        String key = "tank_name_" + String(i);
        preferences.putString(key.c_str(), tanks[i].name);
    }
   // preferences.end();
}