#ifndef NETWORK_H_
#define NETWORK_H_

#include "Globals.h"

bool setupNetwork();

void doNetwork(PZEM_004T_Sensor_t &PZEM004data);
bool openAccessPoint(bool useDefaultValues=false);

#endif // NETWORK_H_