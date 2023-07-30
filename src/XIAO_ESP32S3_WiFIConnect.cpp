// https://wiki.seeedstudio.com/xiao_esp32s3_wifi_usage/#scan-nearby-wifi-networks

#include <Arduino.h>
#include "WiFi.h"

// Replace with your network credentials
const char *ssid     = "_WLAN_";
const char *password = "";

void initWiFi()
{
     WiFi.setMinSecurity(WIFI_AUTH_WEP);   // WIFI_AUTH_OPEN is the minimum
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(1000);
    }
    Serial.println();
    Serial.println(WiFi.localIP());
}

void setup()
{
    Serial.begin(115200);
    delay(5000);
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    initWiFi();
}

void loop() {}