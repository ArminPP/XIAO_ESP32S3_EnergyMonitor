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

enum BlinkingStyle
{
    BS_normalBlink,
    BS_noBlink,
    BS_fastBlink,
    BS_slowBlink
};

void doLedBlink(uint8_t LedPin, BlinkingStyle BS = BS_normalBlink, uint32_t BlinkingSpeed_ms = ONE_SECOND); // blink with default values

// Touch button settings
uint16_t ButtonThreshold = 1500; // ESP32S2
bool ButtonResetEnergy   = false;
uint8_t ButtonResetPin   = TOUCH_PAD_NUM9;

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

// **********************************************************************
// in main loop the led blinks with default freuency as a heart beat (eg. 1s)
// blinks the internal LED for a specific time very fast
// **********************************************************************
void doLedBlink(uint8_t LedPin, BlinkingStyle BS, uint32_t BSpeed_ms)
{
    static boolean blink               = true;
    static boolean NoBlink             = false;
    static boolean FastSlowBlink       = false;
    static uint16_t BlinkCounter_s     = 0;
    static uint16_t BlinkingDuration_s = 0;
    static uint32_t BlinkingSpeed_ms   = 0;

    static unsigned long BlinkLoopPM = 0;
    unsigned long BlinkLoopCM        = millis();

    if (!FastSlowBlink) // only if FastSlowBlink is not set, switch to default!
    {
        BlinkingSpeed_ms = BSpeed_ms; // set default, noBlink and normalBlink need both 1 second blinking
    }

    // BS and BSpeed_ms are dynamic vars, once the function is called
    // they will are gone to the default values during the next loop cycle.
    // So the values are stored to static vars and after a specific time
    // (BlinkingDuration_s) everything will switch back to their defaults.
    if (BS == BS_noBlink)
    {
        NoBlink            = true;
        BlinkingDuration_s = 5;
    }
    else if (BS == BS_fastBlink)
    {
        FastSlowBlink      = true;
        BlinkingSpeed_ms   = 100;
        BlinkingDuration_s = 5 * 10; // duration * related to 1 second
    }
    else if (BS == BS_slowBlink)
    {
        FastSlowBlink      = true;
        BlinkingSpeed_ms   = 2000;
        BlinkingDuration_s = 5; // blink x-times in slow mode
    }

    if (NoBlink || FastSlowBlink) // switch to "notNormal" mode
    {
        // Serial.println("****************************      NoBlink triggered");
        // trigger the no blink event
        if (BlinkCounter_s >= BlinkingDuration_s) // if duration is reached..
        {                                         //
            NoBlink          = false;             // stop the light
            FastSlowBlink    = false;
            BlinkingSpeed_ms = BSpeed_ms; // back to default
            BlinkCounter_s   = 0;
            LOG(LOG_INFO, "NoBlink end");
        }
    }

    if (BlinkLoopCM - BlinkLoopPM >= BlinkingSpeed_ms)
    {
        if ((blink = !blink) || NoBlink)
        {
            digitalWrite(LedPin, LOW);    // turn the LED on (chek if LOW==On or HIGH==ON!!)
            if (NoBlink || FastSlowBlink) // only if not normal Blink than count
            {
                BlinkCounter_s++;
                LOG(LOG_DEBUG, "BlinkCounter_s %i | Duration_s %i | BlinkingSpeed_ms %i", BlinkCounter_s, BlinkingDuration_s, BlinkingSpeed_ms);
            }
        }
        else
        {
            digitalWrite(LedPin, HIGH); // turn the LED off
        }
        // -------- BlinkLoop end ----------------------------------------------------------------
        BlinkLoopPM = BlinkLoopCM;
    }
}

void Button1()
{
    // for each interrupt a button function
    ButtonResetEnergy = true;
}

void setup()
{
    Serial.begin(115200);
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);

    touchAttachInterrupt(ButtonResetPin, Button1, ButtonThreshold); // energy reset button

    delay(2000); // delay is mandatory with Xiao ESP32S3 ?!
    LOG(LOG_INFO, "Welcome to %s\n", ___FILENAME___);
    delay(1000);

    ESP32sysInfo();

    LOG(LOG_INFO, "PZEM004Tv30 Address: %02X", pzem.readAddress()); // HEX address

    setupNetwork();
}

// the loop function runs over and over again forever
void loop()
{
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

    doNetwork(PZEM004Data);  // only webserver on Xiao!
    doLedBlink(LED_BUILTIN); // default blinking

    if (ButtonResetEnergy)
    {
        ButtonResetEnergy = false;
        if (touchInterruptGetLastStatus(ButtonResetPin))
        {
            if (pzem.resetEnergy())
            {
                LOG(LOG_INFO, "Reset PZEM-004 energy readings was succesful");
                doLedBlink(LED_BUILTIN, BS_noBlink); // LED on for 5 seconds
            }
            else
            {
                LOG(LOG_ERROR, "Reset PZEM-004 energy readings not succesful!");
                doLedBlink(LED_BUILTIN, BS_fastBlink); // LED flashes fast for 5 seconds
            }
        }
        else
        {
            // LOG(LOG_DEBUG, "Button reset energy released");
        }
    }

    /*
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
    */
}
