//TESTING REQUIRED
//ADD IN LIBRARY
#ifndef FAVORIOT_CAR_H
#define FAVORIOT_CAR_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class FavoriotCar {
private:
    const char* _ssid;
    const char* _password;
    const char* _apiKey;
    const char* _deviceID;
    const char* _serverName;
    unsigned long _lastTime;
    unsigned long _timerDelay;

public:
    // Constructor
    FavoriotCar(const char* ssid, const char* password, const char* apiKey, const char* deviceID, unsigned long timerDelayMs = 3000);
    
    // Initialize WiFi connection
    void begin();
    
    // Send data to Favoriot platform (Call this inside loop())
    void sendTelemetry(int wFL, int wFR, int wRL, int wRR, String direction, float battery);
};

#endif
