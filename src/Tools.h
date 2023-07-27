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

#ifndef TOOLS_H_
#define TOOLS_H_

// #define USE_M5ATOM
// #define USE_M5CORE
// #define USE_M5CORE2

// #define USE_TFT // if display is available

#ifdef USE_M5ATOM
#include <M5Atom.h>
#endif

#ifdef USE_CORE
#include <M5Stack.h>
#endif

#ifdef USE_CORE2
#include <M5Core2.h>
#endif

#include <Arduino.h>
// #include <Ethernet.h>
// #include <WiFi.h>
// #include "TimeFunction.h"
// #include "Messages.h"

#define ___FILENAME___ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define MAX_FILE_LENGTH 35

void ElapsedRuntime(uint16_t &dd, byte &hh, byte &mm, byte &ss, uint16_t &ms); // used if no NTP, RTC ... is available

 
void WAIT_ATOM(); // M5Atom Tool - push a button to start device!
void LOCK_ATOM(); // M5Atom Tool - Lock device and blink red!
 

// void I2Cscanner(TwoWire &wire);

char *getAllFreeHeap();
char *getUnusedStack();

bool WIFIisConnected();

bool str_iseq(const char *s1, const char *s2); // used for aWOT basic authentication

void ESP32sysInfo(bool Debug = true); // TODO make it better! Is only a prototype at the moment!

char *GetMACquadruple();
#endif               // TOOLS_H_