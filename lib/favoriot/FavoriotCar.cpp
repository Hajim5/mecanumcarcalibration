//TESTING REQUIRED
//ADD IN LIBRARY
#include "FavoriotCar.h"

FavoriotCar::FavoriotCar(const char* ssid, const char* password, const char* apiKey, const char* deviceID, unsigned long timerDelayMs) {
    _ssid = ssid;
    _password = password;
    _apiKey = apiKey;
    _deviceID = deviceID;
    _timerDelay = timerDelayMs;
    _serverName = "https://apiv2.favoriot.com/v2/streams";
    _lastTime = 0;
}

void FavoriotCar::begin() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(_ssid, _password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi network successfully!");
}

void FavoriotCar::sendTelemetry(int wFL, int wFR, int wRL, int wRR, String direction, float battery) {
    // Check if it's time to send data
    if ((millis() - _lastTime) > _timerDelay) {
        if (WiFi.status() == WL_CONNECTED) {
            HTTPClient http;
            http.begin(_serverName);
            
            http.addHeader("Content-Type", "application/json");
            http.addHeader("apikey", _apiKey);
            
            // Allocate JSON document
            StaticJsonDocument<300> doc;
            doc["device_developer_id"] = _deviceID;
            
            JsonObject data = doc.createNestedObject("data");
            data["wheel_fl"] = wFL;
            data["wheel_fr"] = wFR;
            data["wheel_rl"] = wRL;
            data["wheel_rr"] = wRR;
            data["direction"] = direction;
            data["battery"] = String(battery, 2); // Format float to 2 decimal places

            String requestBody;
            serializeJson(doc, requestBody);
            
            int httpResponseCode = http.POST(requestBody);
            
            if (httpResponseCode > 0) {
                Serial.print("Favoriot Update Success! Response code: ");
                Serial.println(httpResponseCode);
            } else {
                Serial.print("Favoriot Error code: ");
                Serial.println(httpResponseCode);
            }
            http.end();
        } else {
            Serial.println("Telemetry Failed: WiFi Disconnected");
        }
        _lastTime = millis();
    }
}
