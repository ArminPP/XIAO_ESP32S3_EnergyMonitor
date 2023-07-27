#ifndef GLOBALS_H_
#define GLOBALS_H_

// ************************************
// global project constants declaration
// https://stackoverflow.com/a/53352579
// ************************************

#include <Arduino.h>

#define HALT for (;;) // endless loop == HALT!

#define ONE_SECOND 1000
#define ONE_MINUTE 1000 * 60
#define ONE_HOUR 1000 * 60 * 60

extern const char *SETTINGS_FILENAME; // LittleFS - must not be set in web server app!

extern const uint16_t SCHEDULER_LOOP; // 1000ms - main loop of the scheduler (read temperatures and so on...)

// EmonCMS Settings
extern const char *EmonHost;
extern const char *EmonNodeID;
extern const char *EmonApiKey;
extern const uint16_t EMONCMS_LOOP_TIME;

// PZEM-004T data

struct PZEM_004T_Sensor_t
{
    float Voltage;
    float Current;
    float Power;
    float Energy;
    float Frequency;
    float PowerFactor;
};

#endif // GLOBALS_H_