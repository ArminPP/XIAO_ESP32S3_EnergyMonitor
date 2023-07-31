#include <Arduino.h>
#include <PZEM004Tv30.h> // https://github.com/mandulaj/PZEM-004T-v30

#include "Globals.h"
#include "Log.h"
#include "Communication.h"
#include "Tools.h"
#include "Network.h"

/*
// INFO                      Xiao ESp32S3                                         .
// INFO                      Xiao ESp32S3                                         .

THE NEW ESP32S3 HAS NOT THE LINEARITY PROBLEM OF HIS PREDECESSOR!
    SO THEORTICALLY IT IS NOT NEEDED THO CALIBRATE THE ADC!
    https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32s3/api-reference/peripherals/adc.html#adc-calibration

    The calibration sofware is using a ADC as reference output for
    the ADC BUT THE ESP32S3 doesn't hav an ADC anymore !!!!!!!!!!
    https://github.com/e-tinkers/esp32-adc-calibrate

Reading an analog value with the ESP32 means you can measure varying voltage levels between 0 V and 3.3 V


CoolTerm is working better with Xiao ESp32S3 ?!

// INFO                     PZEM004Tv30                                          .
// INFO                     PZEM004Tv30                                          .

Modification if PZEM004Tv30 from 5V TTL to 3.V TTL is NOT needed!
    https://tasmota.github.io/docs/PZEM-0XX/#pzem-004t-version-v3_1  // Different V3.3 Mods for V1.0 and V3.0 !!!!!!!!!!!!!!!!!!!!!!!!!!!
    https://github.com/arendst/Tasmota/issues/9518 // explanation !

*/
#define PZEM_RX_PIN GPIO_NUM_44 // --> TX PIN OF PZEM
#define PZEM_TX_PIN GPIO_NUM_43 // --> RX PIN OF PZEM
#define PZEM_SERIAL Serial2

// This device communicates only if 240VAC is connested!!
PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);

// **********************************************************************
// read all data from PZEM-004T
// **********************************************************************
void readPZEM004Data(PZEM_004T_Sensor_t &PZEM004data)
{
    PZEM004data.Voltage     = pzem.voltage();
    PZEM004data.Current     = pzem.current();
    PZEM004data.Power       = pzem.power();
    PZEM004data.Energy      = pzem.energy();
    PZEM004data.Frequency   = pzem.frequency();
    PZEM004data.PowerFactor = pzem.pf();

    LOG(LOG_DEBUG,
        "PZEM_004T   Voltage: %.2f V | Current %.2f A  | Power %.2f W  | Energy %.2f kWh  | Frequency %.2f Hz  | PowerFactor %.2f",
        PZEM004data.Voltage,
        PZEM004data.Current,
        PZEM004data.Power,
        PZEM004data.Energy,
        PZEM004data.Frequency,
        PZEM004data.PowerFactor);
}

void setup()
{
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
    delay(2000); // delay is mandatory with Xiao ESP32S3 ?!
    Serial.printf("Welcome to %s\n", ___FILENAME___);
    delay(1000);

    ESP32sysInfo();

    Serial.print("PZEM004Tv30 Address: ");
    Serial.println(pzem.readAddress(), HEX);

    setupNetwork();
}

// the loop function runs over and over again forever
void loop()
{
    static bool blink = true;

    PZEM_004T_Sensor_t PZEM004Data;

    static unsigned long EmonCmsLoopPM = 0;
    unsigned long EmonCmsLoopCM        = millis();
    if (EmonCmsLoopCM - EmonCmsLoopPM >= (EMONCMS_LOOP_TIME * ONE_SECOND))
    {
        // Read the data from the sensor

        readPZEM004Data(PZEM004Data);
        sendToEMON(PZEM004Data);

        // -------- EmonCmsLoop end ----------------------------------------------------------------
        EmonCmsLoopPM = EmonCmsLoopCM;
    }

    doNetwork(PZEM004Data); // only webserver on Xiao!

    static unsigned long oneSecondLoopPM = 0;
    unsigned long oneSecondLoopCM        = millis();
    if (oneSecondLoopCM - oneSecondLoopPM >= (1 * ONE_SECOND))
    {
        if ((blink = !blink))
        {
            digitalWrite(LED_BUILTIN, HIGH); // turn the LED on
        }
        else
        {
            digitalWrite(LED_BUILTIN, LOW); // turn the LED off
        }

        // -------- oneSecondLoop end ----------------------------------------------------------------
        oneSecondLoopPM = oneSecondLoopCM;
    }

    const int CAPACITIVE_TOUCH_INPUT_PIN = GPIO_NUM_4; // GPIO pin 14
    const int TOUCH_THRESHOLD            = 20;         // turn on light if touchRead value < this threshold

    unsigned long startReadTimestamp = micros();
    // By default, touchRead should take ~500 microseconds according to the ESP32 source code
    // https://github.com/espressif/arduino-esp32/blob/a59eafbc9dfa3ce818c110f996eebf68d755be24/cores/esp32/esp32-hal-touch.h
    // You can configure these settings using touchSetCycles
    int touchVal              = touchRead(CAPACITIVE_TOUCH_INPUT_PIN);
    unsigned long elapsedTime = micros() - startReadTimestamp;
    boolean ledOn             = false;
// xxx


    // Turn on LED if touchRead value drops below threshold
    if (touchVal < TOUCH_THRESHOLD)
    {
        ledOn = true;
        Serial.println((String)touchVal + ", " + elapsedTime + " microseconds, LED " + ledOn);
    }
}