#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>
#include <FastLED.h>
#include <Scheduler.h>
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

const char serverAddress[] = "api.tidesandcurrents.noaa.gov";  // server address
// const char serverAddress1[] = "api.stormglass.io";
int port = 443;
String station = "8518750";  //station ID: The Battery, NY

// make a wifi instance and a HttpClient instance:
WiFiSSLClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
// HttpClient client1 = HttpClient(wifi, serverAddress1, port);
int status = WL_IDLE_STATUS;

// const long requestInterval = 10000; // 6 minutes;
// long lastRequestTime = 0;

float tide;

#define NUM_LEDS_PER_STRIP 49

#define NUM_STRIPS 2
#define FASTLED_INTERNAL

CRGB leds[NUM_STRIPS][NUM_LEDS_PER_STRIP];
uint8_t paletteIndex = 0;

DEFINE_GRADIENT_PALETTE(redblue_gp){
  0, 255, 255, 255,
  153, 119, 173, 209,
  184, 211, 56, 67,
  219, 255, 0, 0,
  255, 103, 3, 12
};

CRGBPalette16 RED = redblue_gp;


void setup() {

  Serial.begin(57600);

  connectWifi();

  delay(100);
  Scheduler.startLoop(loop1);
  FastLED.addLeds<WS2812, 8, GRB>(leds[0], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812, 9, GRB>(leds[0], NUM_LEDS_PER_STRIP);
  FastLED.setBrightness(50);
  FastLED.clear(true);

  getData();
}

void loop() {

  // if (millis() - lastRequestTime > requestInterval) {
  getData();
  // }
}

void loop1() {
  //////////////////////////////////////////////////////////////////////////////////////

  uint16_t beatA = beatsin16(10, 0, 255);
  uint16_t beatB = beatsin16(5, 0, 255);
  uint16_t paletteIndex = (beatA + beatB) / 2;
  // uint16_t paletteIndex = beatA / 2;

  int ledLev = map(tide, 0, 7, 0, 48);
  // // Serial.println(ledLev);

  for (int x = 0; x < NUM_STRIPS; x++) {
    for (int thisLed = 0; thisLed < NUM_LEDS_PER_STRIP; thisLed = thisLed + 1) {

      for (thisLed = ledLev; thisLed < ledLev + 8; thisLed++) {
        fill_palette(leds[x], thisLed + 1, paletteIndex, 10, RED, 255, LINEARBLEND);
      }

      for (thisLed = 0; thisLed < ledLev; thisLed++) {
        leds[x][thisLed] = CRGB(255, 0, 0);
      }

      for (thisLed = ledLev + 7; thisLed < NUM_LEDS_PER_STRIP; thisLed++) {
        leds[x][thisLed] = CRGB(255, 255, 255);
      }
    }
  }

  EVERY_N_MILLISECONDS(10) {
    paletteIndex++;
  }

  FastLED.show();

  yield();
}

void getData() {

  String path = "/api/prod/datagetter?date=latest&station=" + station + "&product=water_level&datum=MLLW&time_zone=lst_ldt&units=english&format=json";


  // send the GET request
  Serial.println("making GET request");
  // Serial.println(path);

  client.get(path);

  // read the status code and body of the response
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();



  Serial.print("Status code: ");
  Serial.println(statusCode);

  // Serial.print("Response: ");
  // Serial.println(response);

  // parse the response to get the value of aqius
  int labelStart = response.indexOf("v");
  int valueStart = response.indexOf(":", labelStart);
  int valueEnd = response.indexOf(",", labelStart);
  String valueS = response.substring(valueStart + 2, valueEnd - 1);

  // //Serial.print(valueS);

  // // convert the string to int
  tide = valueS.toFloat();
  Serial.println(tide);
  delay(360000);

  // client.stop();
  // lastRequestTime = millis();
}

void connectWifi() {

  // while (!Serial)
  //   ;
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);  // print the network name (SSID);

    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
  }

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}