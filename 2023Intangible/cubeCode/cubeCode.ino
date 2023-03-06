#include <FastLED.h>
#include <Wire.h>
#define analogPinForRV A1  // change to pins you the analog pins are using
#define analogPinForTMP A0

#include <Servo.h>
#include "Adafruit_VL6180X.h"

Adafruit_VL6180X vl = Adafruit_VL6180X();

Servo myServo;
byte servoPin = 2;

// to calibrate your sensor, put a glass over it, but the sensor should not be
// touching the desktop surface however.
// adjust the zeroWindAdjustment until your sensor reads about zero with the glass over it.

const float zeroWindAdjustment = .2;  // negative numbers yield smaller wind speeds and vice versa.

int TMP_Therm_ADunits;  //temp termistor value from wind sensor
float RV_Wind_ADunits;  //RV output from wind sensor
float RV_Wind_Volts;
unsigned long lastMillis;
int TempCtimes100;
float zeroWind_ADunits;
float zeroWind_volts;
float WindSpeed_MPH;


//
// Move a white dot along the strip of leds.  This program simply shows how to configure the leds,
// and then how to turn a single pixel white and then off, moving down the line of pixels.
//

// How many leds are in the strip?
#define NUM_LEDS 60

// For led chips like WS2812, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// Clock pin only needed for SPI based chipsets when not using hardware SPI
#define DATA_PIN 3
#define CLOCK_PIN 13

#define BRIGHTNESS 20

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];


// This function sets up the ledsand tells the controller about them
void setup() {
  Serial.begin(115200);
  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  delay(2000);

  // wait for serial port to open on native usb devices
  while (!Serial) {
    delay(1);
  }

  Serial.println("Adafruit VL6180x test!");
  if (!vl.begin()) {
    Serial.println("Failed to find sensor");
    while (1)
      ;
  }
  Serial.println("Sensor found!");

  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);  // GRB ordering is typical

    myServo.attach(servoPin);

  Serial.println("start");
  // put your setup code here, to run once:

  //   Uncomment the three lines below to reset the analog pins A2 & A3
  //   This is code from the Modern Device temp sensor (not required)
  pinMode(A2, INPUT);     // GND pin
  pinMode(A3, INPUT);     // VCC pin
  digitalWrite(A3, LOW);  // turn off pullups
}

void loop() {
  //distance sensor:
  uint8_t range = vl.readRange();
  Serial.print("Range: ");
  Serial.println(range);
  delay(50);
  if (range <= 180) {
    // your leds.
    for (int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
      // Turn our current led on to white, then show the leds
      leds[whiteLed] = CRGB::White;
      FastLED.setBrightness(BRIGHTNESS);
      // Show the leds (only one of which is set to white, from above)
      FastLED.show();
    }
  }
  if (range >= 200) {
    for (int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
      // Turn our current led on to white, then show the leds
      leds[whiteLed] = CRGB::Black;
      //FastLED.setBrightness(BRIGHTNESS);
      // Show the leds (only one of which is set to white, from above)
      FastLED.show();
    }
}

 if (millis() - lastMillis > 200) {  // read every 200 ms - printing slows this down further

    TMP_Therm_ADunits = analogRead(analogPinForTMP);
    RV_Wind_ADunits = analogRead(analogPinForRV);
    RV_Wind_Volts = (RV_Wind_ADunits * 0.0048828125);

    // these are all derived from regressions from raw data as such they depend on a lot of experimental factors
    // such as accuracy of temp sensors, and voltage at the actual wind sensor, (wire losses) which were unaccouted for.
    TempCtimes100 = (0.005 * ((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits)) - (16.862 * (float)TMP_Therm_ADunits) + 9075.4;

    zeroWind_ADunits = -0.0006 * ((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits) + 1.0727 * (float)TMP_Therm_ADunits + 47.172;  //  13.0C  553  482.39

    zeroWind_volts = (zeroWind_ADunits * 0.0048828125) - zeroWindAdjustment;

    // This from a regression from data in the form of
    // Vraw = V0 + b * WindSpeed ^ c
    // V0 is zero wind at a particular temperature
    // The constants b and c were determined by some Excel wrangling with the solver.

    WindSpeed_MPH = pow(((RV_Wind_Volts - zeroWind_volts) / .2300), 2.7265);

    Serial.print("   WindSpeed MPH ");
    Serial.println((float)WindSpeed_MPH);
    
    //map the windSpeed to servoSpeed:
    //Servo Speed: 1440-1450:stop, 1450>:clockwise, range:1350-1440
    //Wind Speed: 50-170
    int servoSpeed=map((float)WindSpeed_MPH,170,50,1350,1440);
    myServo.writeMicroseconds(servoSpeed);
    Serial.print("   ServoSpeed ");
    Serial.println(servoSpeed);
    
      lastMillis = millis();
  }
}
