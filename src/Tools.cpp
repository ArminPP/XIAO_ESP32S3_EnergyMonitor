/**
 * @file     Tools.h
 * @author   Armin P Pressler (app@gmx.at)
 * @brief    - adaption to M5Atom (no SD, no RTC, no TFT switches with #define)
 * @version  0.3
 * @date     2022-08-30
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "Tools.h"

#include <Arduino.h>
#include <WiFi.h>

#include "Log.h"

// #ifdef USE_M5ATOM
// #include <M5Atom.h>
// #endif
// #ifdef USE_CORE
// #include <M5Stack.h>
// #endif
// #ifdef USE_CORE2
// #include <M5Core2.h>
// #endif

#ifdef USE_M5ATOM
void WAIT_ATOM() // push a button to start device!
{
  while (Serial.available() == 0)
  {
    M5.update();
    if (M5.Btn.wasPressed())
    { // if M5 Button was pressed, then also start...
      break;
    }
  }
}
#endif

#ifdef USE_M5ATOM
void LOCK_ATOM() // Lock device and blink red!
{
  static bool blink = false;
  while (true)
  { // endless loop!
    yield();
    delay(250);
    if ((blink = !blink))
    {
      M5.dis.drawpix(0, 0x000000); // blue
    }
    else
    {
      M5.dis.drawpix(0, 0xff0000); // red
    }
  }
}
#endif

void printHex(uint8_t address)
// helper for I2Cscanner
{
  Serial.print(" ");

  if (address < 16)
  {
    Serial.print("0");
  }
  Serial.print(address, HEX);
}
/*
void I2Cscanner(TwoWire &wire)
{
  uint8_t error, address, line = 1;
  int nDevices = 0;

  Serial.println("\n     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
  Serial.print("00:         ");

#ifdef USE_TFT
  TFTterminal.println();
  TFTterminal.println("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
  TFTterminal.print("00:         ");
#endif

  // https://learn.adafruit.com/i2c-addresses/the-list
  for (address = 0x03; address < 0x78; address++)
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    wire.beginTransmission(address);
    error = wire.endTransmission();

    if (error == 0)
    {
      printHex(address);
      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print(" ER");
#ifdef USE_TFT
      TFTterminal.print(" ER");
#endif
    }
    else
    {
      Serial.print(" --");
#ifdef USE_TFT
      TFTterminal.print(" --");
#endif
    }
    if ((address + 1) % 16 == 0)
    {
      Serial.println();
      Serial.print(line);
      Serial.print("0:");
#ifdef USE_TFT
      TFTterminal.println();
      TFTterminal.print(line);
      TFTterminal.print("0:");
#endif
      line++;
    }
  }
  if (nDevices == 0)
  {
    Serial.println("\nNo I2C devices found\n");
#ifdef USE_TFT
    TFTterminal.print("\nNo I2C devices found\n");
#endif
  }
  else
  {
    Serial.print("\nFound ");
    Serial.print(nDevices);
    Serial.println(" devices\n");
#ifdef USE_TFT
    TFTterminal.print("\nFound ");
    TFTterminal.print(nDevices);
    TFTterminal.println(" devices\n");
#endif
  }
}
*/

char *getAllFreeHeap()
// returns the formated free heap space
//
// getMinFreeHeap:
// In the same file, if you read the header of the esp_get_minimum_free_heap_size, it says the following:
// “Get the minimum heap that has ever been available”.
// So that function has kind of a misleading name, but it seems that it returns the minimum value ever available.
// Looks like a 'Schleppzeiger', shows always the actual lowest size since boot - and not more!
{
  static char freeH[80]{}; // this costs memory, like a global var!
  // https://forum.arduino.cc/t/can-a-function-return-a-char-array/63405/5
  sprintf(freeH, "Size: %.2f kB Free: %.2f kB Min: %.2f kB Max: %.2f kB",
          ESP.getHeapSize() / 1024.0,
          ESP.getFreeHeap() / 1024.0,
          ESP.getMinFreeHeap() / 1024.0,
          ESP.getMaxAllocHeap() / 1024.0);
  return freeH;
}

char *getUnusedStack()
// returns the formated unused stack space in bytes!
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html?highlight=highwatermark
{
  static char unUsedS[80]{}; // this costs memory, like a global var!
  // https://forum.arduino.cc/t/can-a-function-return-a-char-array/63405/5
  snprintf(unUsedS, sizeof(unUsedS), "HiWaterMark: %d bytes | Idle: %d bytes",
           uxTaskGetStackHighWaterMark(0),
           uxTaskGetStackHighWaterMark(xTaskGetIdleTaskHandle()));
  return unUsedS;
}

void ElapsedRuntime(uint16_t &dd, byte &hh, byte &mm, byte &ss, uint16_t &ms)
// returns the elapsed time since startup of the ESP
{
  unsigned long now = millis();
  int nowSeconds = now / 1000;

  dd = nowSeconds / 60 / 60 / 24;
  hh = (nowSeconds / 60 / 60) % 24;
  mm = (nowSeconds / 60) % 60;
  ss = nowSeconds % 60;
  ms = now % 1000;
}

bool str_iseq(const char *s1, const char *s2)
// https://nachtimwald.com/2017/04/02/constant-time-string-comparison-in-c/
// Constant Time String Comparison in C
// Instead of strncmp which is susceptible to timing attacks because it will stop comparing once the first difference is encountered
{
  int m = 0;
  size_t i = 0;
  size_t j = 0;
  size_t k = 0;
  if (s1 == NULL || s2 == NULL)
  {
    return false;
  }
  while (true)
  {
    m |= s1[i] ^ s2[j];
    if (s1[i] == '\0')
    {
      break;
    }
    i++;
    if (s2[j] != '\0')
    {
      j++;
    }
    if (s2[j] == '\0')
    {
      k++;
    }
  }
  return m == 0;
}

// TODO make it better
// TODO is only a  prototype at the moment!
void ESP32sysInfo(bool Debug)
// **********************************************************************
// **********************************************************************
{
  if (Debug == true)
  {
    LOG(LOG_DEBUG, "CPU Freq = %i MHz", getCpuFrequencyMhz());
    LOG(LOG_DEBUG, "XTAL Freq = %i MHz", getXtalFrequencyMhz());
    LOG(LOG_DEBUG, "APB Freq = %i MHz", getApbFrequency());
  }
  else
  {
    Serial.print("CPU Freq = ");
    Serial.print(getCpuFrequencyMhz());
    Serial.println(" MHz");

    Serial.print("XTAL Freq = ");
    Serial.print(getXtalFrequencyMhz());
    Serial.println(" MHz");

    Serial.print("APB Freq = ");
    Serial.print(getApbFrequency());
    Serial.println(" Hz");
  }
  // BUG    On M5Stack core this function kills the M5 lib    !?
  // BUG    eg. it always jumps to AP function, don't know why?!
  // BUG    ####################################################
  // Serial.print("Hall Sensor = ");
  // Serial.print(hallRead());
  // Serial.println(".");

  // INFO on classic ESP32 (wroom) the sensor is not active!
  // https://github.com/espressif/arduino-esp32/issues/2422
  // Serial.print("Core Temp = ");
  // Serial.print((temprature_sens_read() - 32) / 1.8);
  // Serial.println(" °C");

  /* Print chip information */
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  // TODO    include printf also to LOG !!!!!!!!!!!!!!!!!!!!!!
  printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
         chip_info.cores,
         (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
         (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

  printf("silicon revision %d, ", chip_info.revision);

  printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
         (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

  if (Debug == true)
  {
    LOG(LOG_DEBUG, getAllFreeHeap());
    LOG(LOG_DEBUG,getUnusedStack());
  }
  else
  {
    Serial.println(getAllFreeHeap());
    Serial.println(getUnusedStack());
  }
  //  SDCARDINFO ?!?!?!?!?!?  // TODO status of SDCard and SPIFFS!
}


// check if WiFi is connected
bool WIFIisConnected()
// **********************************************************************
// **********************************************************************
{
  if (WiFi.status() == WL_CONNECTED)
  {

#ifdef DEBUG
    WiFi.printDiag(Serial);
#endif

    return true; // exit setup, because WiFi is connected
  }
  else
  {
    return false;
  }
}

char *GetMACquadruple()
// **********************************************************************
// **********************************************************************
{
  uint8_t baseMac[6];
  // Get MAC address for WiFi station
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  static char quadrupleMacChr[5] = {'\0'}; // this costs memory, like a global var!
  // https://forum.arduino.cc/t/can-a-function-return-a-char-array/63405/5
  snprintf(quadrupleMacChr, sizeof(quadrupleMacChr), "%02X%02X", baseMac[4], baseMac[5]);

  Serial.print(F("last quadruple Mac adress: "));
  Serial.println(quadrupleMacChr);

  return quadrupleMacChr;
}