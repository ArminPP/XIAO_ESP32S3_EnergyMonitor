/**
 * @file     Credentials.h
 * @author   Armin P Pressler (app@gmx.at)
 * @brief    - adaption to M5Atom (basic set of credentials, no save to SPIFFS!)
 * @version  0.3
 * @date     2022-08-30
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <Arduino.h>
#include "Credentials.h"

namespace Credentials // https://stackoverflow.com/questions/2483978/best-way-to-implement-globally-scoped-data
{
// --- Global ---
bool RESTART_ESP = false; // not stored in aWOT, only global var!

// --- Logfile ---
bool LOG_TO_SERIAL = true;

bool ENABLE_LOG_DEBUG   = true;
bool ENABLE_LOG_ERROR   = true;
bool ENABLE_LOG_WARNING = true;
bool ENABLE_LOG_INFO    = true;

// --- Mesurement ---
// --- Communication Settings ---
bool AP_ENABLED   = false; // only for webserver (settings, fast measurement,...)
bool WIFI_ENABLED = true;  // only for webserver (settings, fast measurement,...)

// --- MESH ---
// --- MODBUS ---

// --- EMONCMS ---
char EMONCMS_HOST[25]      = "192.168.0.xxx";
uint16_t EMONCMS_PORT      = 80;
char EMONCMS_NODE_ID[25]   = "SolarDuino";
char EMONCMS_API_KEY[35]   = ""; // Your RW apikey
uint16_t EMONCMS_LOOP_TIME = 5;                                  // SECONDS!
uint32_t EMONCMS_TIMEOUT   = 1500;                               // MS

// --- Network ---
char ESP_HOSTNAME[35] = "Xiao_PowerMeter";

char AP_SSID[25]      = "Xiao_PowerMeter";
char AP_PASSWORD[25]  = "sdvK30t9zI%"; // minimum 8 chars (Android!)
char AP_STATIC_IP[25] = "10.0.0.15";
char AP_STATIC_SN[25] = "255.255.255.0";

char WIFI_SSID[25]       = "";
char WIFI_PASSWORD[25]   = "";
bool WIFI_DHCP           = true;
char WIFI_STATIC_IP[25]  = "";
char WIFI_STATIC_SN[25]  = "";
char WIFI_STATIC_GW[25]  = "";
char WIFI_STATIC_DNS[25] = "";

} // namespace Credentials