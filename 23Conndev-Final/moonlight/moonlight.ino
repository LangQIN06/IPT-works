#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include <FastLED.h>
#include <Scheduler.h>
#include <Servo.h>

#define NUM_LEDS 56

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
CRGB leds[NUM_LEDS];

const char serverAddress[] = "api.stormglass.io";  // server address
int port = 443;
String API = "9602c416-dc8e-11ed-bc36-0242ac130002-9602c498-dc8e-11ed-bc36-0242ac130002";  // get your api at aqair.com
//376d28fc-dd55-11ed-a138-0242ac130002-376d296a-dd55-11ed-a138-0242ac130002

// const long requestInterval = 120000;  //1 hour
// long lastRequestTime = 0;

// make a wifi instance and a HttpClient instance:
WiFiSSLClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;

Servo myServo;
byte servoPin = 10;

int pos;
float valueS2;
String valueS;

void setup() {

  Serial.begin(9600);

  connectWifi();
  myServo.attach(servoPin);
  Scheduler.startLoop(loop1);

  FastLED.addLeds<WS2812B, 7, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(50);

  for (int i = 0; i < 56; i = i + 1) {
    leds[i] = CRGB(255, 232, 145);
    ;
    FastLED.setBrightness(10);
    FastLED.show();
  }

  getData();
}

void loop() {

  // if (millis() - lastRequestTime > requestInterval) {
  getData();
  // }
  // delay(2000);
}

void loop1() {
  // servo:
  for (int pos = 0; pos <= 180; pos += 1) {
    myServo.write(pos);
    //Serial.println(pos);
    delay(50);
  }

  delay(500);

  for (int pos = 180; pos >= 0; pos -= 1) {
    myServo.write(pos);
    // Serial.println(pos);
    delay(50);
  }

  delay(500);

  yield();
}

void getData() {


  String path = "/v2/astronomy/point?key=" + API + "&lat=40&lng=-74";

  // Serial.println("making GET request");

  client.get(path);

  // read the status code and body of the response
  int statusCode1 = client.responseStatusCode();
  String response1 = client.responseBody();

  // Serial.print("Status code1: ");
  // Serial.println(statusCode1);
  // Serial.print("Response: ");
  // Serial.println(response);

  // parse the response to get the value of aqius
  int labelStart = response1.indexOf("current");
  int valueStart = response1.indexOf(":", labelStart);
  int valueEnd = response1.indexOf(",", labelStart);
  String valueS = response1.substring(valueStart + 10, valueEnd - 1);
  // Serial.println(valueS);

  int labelStart2 = response1.indexOf("moonFraction");
  int valueStart2 = response1.indexOf(":", labelStart2);
  int valueEnd2 = response1.indexOf(",", labelStart2);
  String valueS2 = response1.substring(valueStart2 + 1, valueEnd2);
  valueS2 = valueS2.toFloat();
  
  Serial.println(valueS2);

  // waxing gibbous
  if (valueS == "Waxing gibbous" && valueS2 >= "0.55" && valueS2 < "0.985") {
    Serial.println("Waxing gibbous!!!");
    for (int i = 35; i < 42; i = i + 1) {
      leds[i] = CRGB::Red;
      FastLED.setBrightness(50);
      FastLED.show();
    }
  }

  // full moon
  if (valueS == "Waning gibbous" && valueS2 >= "0.985") {
    Serial.println("Full moon!!!");
    for (int i = 42; i < 49; i = i + 1) {
      leds[i] = CRGB::Red;
      FastLED.setBrightness(50);
      FastLED.show();
    }
  }

  // Waning gibbous
  if (valueS == "Waning gibbous" && valueS2 >= "0.55" && valueS2 < "0.985") {
    Serial.println("Waning gibbous!!!");
    for (int i = 49; i < 56; i = i + 1) {
      leds[i] = CRGB::Red;
      FastLED.setBrightness(50);
      FastLED.show();
    }
  }

  // Third Quarter
  if (valueS == "Waning gibbous" && valueS2 > "0.45" && valueS2 < "0.55") {
    Serial.println("Third quater!!!");
    for (int i = 0; i < 7; i = i + 1) {
      leds[i] = CRGB::Red;
      FastLED.setBrightness(50);
      FastLED.show();
    }
  }

  // Waning crescent
  if (valueS == "Waning crescent" && valueS2 <= "0.45" && valueS2 >= "0.003") {
    Serial.println("Waning crescent!!!");
    // digitalWrite(2, HIGH);
    for (int i = 7; i < 14; i = i + 1) {
      leds[i] = CRGB::Red;
      FastLED.setBrightness(50);
      FastLED.show();
    }
  }

  // new moon
  if (valueS == "Waxing crescent" && valueS2 < "0.003" ) {

      Serial.println("New moon!!!");
      for (int i = 14; i < 21; i = i + 1) {
        /////////////////////
        // leds[i] = CRGB(255, 232, 145);;
        leds[i] = CRGB::Red;
        FastLED.setBrightness(50);
        FastLED.show();
     
    }
  }

  // Waxing crescent
  if (valueS == "Waxing crescent" && valueS2 <= "0.45" && valueS2 > "0.003") {

      Serial.println("Waxing crescent!!!");
      for (int i = 21; i < 28; i = i + 1) {
        leds[i] = CRGB::Red;
        FastLED.setBrightness(50);
        FastLED.show();
      
    }
  }

  // First Quarter
  if (valueS == "Waxing crescent" && valueS2 > "0.45" && valueS2 < "0.55") {
    Serial.println("First Quarter!!!");
    for (int i = 28; i < 35; i = i + 1) {
      leds[i] = CRGB::Red;
      FastLED.setBrightness(50);
      FastLED.show();
    }
  }

  delay(21600000);
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