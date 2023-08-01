#include "Globals.h"
#include "Credentials.h"

const char *SETTINGS_FILENAME = "/Settings.ini"; // LittleFS  - must not be set in web server app!

const uint16_t SCHEDULER_LOOP = 1000; // 1000ms - main loop of the scheduler (read temperatures and so on...)

// EmonCMS Settings
// const char *Credentials::EMONCMS_HOST = "192.168.0.194";
// const char *Credentials::EMONCMS_NODE_ID = "PowerDuino";
// const char *Credentials::EMONCMS_API_KEY = "1ce596688fc9a1e40d25d855a1336dad"; // Your RW apikey
// const uint16_t EMONCMS_LOOP_TIME = 5;                       // SECONDS!
