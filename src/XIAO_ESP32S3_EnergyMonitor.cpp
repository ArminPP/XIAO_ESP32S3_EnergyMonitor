#include <Arduino.h>

/*
// INFO                                                               .
// INFO                                                               .

THE NEW ESP32S3 HAS NOT THE LINEARITY PROBLEM OF HIS PREDECESSOR!
SO THEORTICALLY IT IS NOT NEEDED THO CALIBRATE THE ADC!
https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32s3/api-reference/peripherals/adc.html#adc-calibration

The calibration sofware is using a ADC as reference output for 
the ADC BUT THE ESP32S3 doesn't hav an ADC anymore !!!!!!!!!!
https://github.com/e-tinkers/esp32-adc-calibrate


Reading an analog value with the ESP32 means you can measure varying voltage levels between 0 V and 3.3 V

// INFO                                                               .
// INFO                                                               .
*/


https://github.com/Savjee/home-energy-monitor
https://savjee.be/blog/Home-Energy-Monitor-ESP32-CT-Sensor-Emonlib/


// the setup function runs once when you press reset or power the board
void setup()
  {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  }

// the loop function runs over and over again forever
void loop()
  {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  Serial.println("BLINK");
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
  }