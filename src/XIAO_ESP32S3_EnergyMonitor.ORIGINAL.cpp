#include <Arduino.h>
#include <PZEM004Tv30.h> // https://github.com/mandulaj/PZEM-004T-v30

#include "Globals.h"
#include "Log.h"
#include "Communication.h"
#include "Tools.h"
#include "Network.h"
#include "ProfileTimer.h"

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

// This device communicates only if 240VAC is connected!!
PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);

// **********************************************************************
// read all data from PZEM-004T
// INFO  reading of PZEM-004T takes ~600ms in BLOCKING mode        !!!!!!
// **********************************************************************
void readPZEM004Data(PZEM_004T_Sensor_t &PZEM004data)
{
    {
        // tests if value is "Nan" (then set to 0.0) or a valid float ...
        // generates "Nan" for testing ==>  float voltage = float(sqrt(-2.0));
        PZEM004data.Voltage     = isnan(pzem.voltage()) ? 0.0 : pzem.voltage();
        PZEM004data.Current     = isnan(pzem.current()) ? 0.0 : pzem.current();
        PZEM004data.Power       = isnan(pzem.power()) ? 0.0 : pzem.power();
        PZEM004data.Energy      = isnan(pzem.energy()) ? 0.0 : pzem.energy();
        PZEM004data.Frequency   = isnan(pzem.frequency()) ? 0.0 : pzem.frequency();
        PZEM004data.PowerFactor = isnan(pzem.pf()) ? 0.0 : pzem.pf();

        ProfileTimer tt("read PZEM004_1");
    }
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
    if (EmonCmsLoopCM - EmonCmsLoopPM >= (Credentials::EMONCMS_LOOP_TIME * ONE_SECOND))
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
        // Serial.println("Touch1 " + (String)touchRead(TOUCH_PAD_NUM1));
        // Serial.println("Touch2 " + (String)touchRead(TOUCH_PAD_NUM2));
        // Serial.println("Touch3 " + (String)touchRead(TOUCH_PAD_NUM3));
        // Serial.println("Touch4 " + (String)touchRead(TOUCH_PAD_NUM4));
        // Serial.println("Touch5 " + (String)touchRead(TOUCH_PAD_NUM5));
        // Serial.println("Touch6 " + (String)touchRead(TOUCH_PAD_NUM6));
        // Serial.println("Touch7 " + (String)touchRead(TOUCH_PAD_NUM7));
        // Serial.println("Touch8 " + (String)touchRead(TOUCH_PAD_NUM8));
        // Serial.println("Touch9 " + (String)touchRead(TOUCH_PAD_NUM9));

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

    // INFO  TOUCH ESP32 != ESP32S3 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Actually, the touch sensor on ESP32-S2 and ESP32-S3 adopts an opposite way to measure the capacity,
    // they will fixed the count of charge and discharge cycles and then record the count of the
    // clock cycles during the sensing period.
    // https://github.com/espressif/esp-idf/issues/9067#issuecomment-1143073486

    const uint32_t TOUCH_THRESHOLD    = 40000;
    const uint16_t ButtonPress        = 300;
    static boolean TriggerOnce        = false;
    static unsigned long previousTime = 0;
    unsigned long currentTime         = millis();

    int touchVal = touchRead(TOUCH_PAD_NUM9);
    if (touchVal > TOUCH_THRESHOLD)
    {
        if (currentTime - previousTime >= ButtonPress && !TriggerOnce)
        {
            Serial.println((String)touchVal + " ####################### PRESSED! ");
            TriggerOnce  = true; // activate trigger after button press is detected
            previousTime = currentTime;
        }
    }
    else
    {
        TriggerOnce = false; // after releasing the button, reset trigger for new button press event
    }
}