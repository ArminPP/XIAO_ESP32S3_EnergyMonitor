#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include <Arduino.h>
#include "Globals.h"

const uint16_t MQTTWiFiTimeout    = 1500; // ms
const uint16_t EmonCmsWiFiTimeout = 5000; // ms

void sendToEMON(PZEM_004T_Sensor_t &PZEM004data);

#endif // COMMUNICATION_H_