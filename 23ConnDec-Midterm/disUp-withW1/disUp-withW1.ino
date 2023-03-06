#include <WiFiNINA.h>  // use this for Nano 33 IoT, MKR1010, Uno WiFi

#include <ArduinoMqttClient.h>

#include <Wire.h>

#include "Adafruit_VL53L0X.h"

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

#include "arduino_secrets.h"

// initialize WiFi connection as SSL:
//WiFiSSLClient wifi;
WiFiClient wifi;
MqttClient mqttClient(wifi);

// details for MQTT client:
char broker[] = "public.cloud.shiftr.io";
//char broker[] = "test.mosquitto.org";
int port = 1883;
String topic = "conndev/lq2078";
char clientID[] = "Client-lang";

// last time the client sent a message, in ms:
long lastTimeSent = 0;
// message sending interval:
int interval = 1 * 1000;
int threshold = 450;
int WiFiPin = 2;
int MQTTPin = 3;
int warnPin = 4;
int warn1Pin = 5;
int warn = 0;
uint16_t dis = 10000;
uint16_t last_dis = 10000;

String macAddr;

void setup() {
  // initialize serial:
  Serial.begin(115200);
  // wait for serial monitor to open:
  if (!Serial) delay(3000);

  pinMode(WiFiPin, OUTPUT);
  //pinMode(conPin, INPUT);
  pinMode(MQTTPin, OUTPUT);
  pinMode(warnPin, OUTPUT);

  // get MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  // put it in a string:
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) macAddr += "0";
    macAddr += String(mac[i], HEX);
  }
  Serial.println();

  // set the credentials for the MQTT client:
  mqttClient.setId(clientID);
  // if needed, login to the broker with a username and password:
  mqttClient.setUsernamePassword(SECRET_MQTT_USER, SECRET_MQTT_PASS);

  Serial.println("Adafruit VL53L0X test");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while (1)
      ;
  }
  // power
  Serial.println(F("VL53L0X API Simple Ranging example\n\n"));
}


void loop() {
  //if you disconnected from the network, reconnect:
  if (WiFi.status() != WL_CONNECTED) {
    connectToNetwork();
    // skip the rest of the loop until you are connected:
    return;
    digitalWrite(WiFiPin, LOW);
  } else {
    digitalWrite(WiFiPin, HIGH);
  }

  // if not connected to the broker, try to connect:
  //if (digitalRead(conPin) == HIGH) {
    if (!mqttClient.connected()) {
      Serial.println("attempting to connect to broker");
      connectToBroker();
    }
  //}
  // poll for new messages from the broker:
  mqttClient.poll();

  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);  // pass in 'true' to get debug data printout!

  // once every interval, send a message:
  if (millis() - lastTimeSent > interval) {
    // read the sensor
    uint16_t dis = measure.RangeMilliMeter;


    if (dis != last_dis) {
      if (dis < threshold) {
        digitalWrite(6, HIGH);  // turn the LED on (HIGH is the voltage level)
        digitalWrite(5, HIGH); 
        digitalWrite(4, HIGH); 
        delay(100);             // wait for a second
        digitalWrite(6, LOW);   // turn the LED off by making the voltage LOW
        digitalWrite(5, LOW);
        digitalWrite(4, LOW);
        delay(100);
        if (last_dis > threshold) {
          warn++;
          Serial.println(warn);
        }
      }
    } else {
      digitalWrite(6, LOW);
      digitalWrite(5, LOW);
      digitalWrite(4, LOW);
    }
    last_dis = dis;
    // make a subtopic for it:

    String subTopic = topic + String("/dis");
    mqttUpdate(subTopic, String(dis));
    if (dis > 1000) {
      mqttUpdate(subTopic, String("leave"));
    }
    // make a subtopic for the other variables too:
    subTopic = topic + String("/warn");
    mqttUpdate(subTopic, String(warn));
    lastTimeSent = millis();
  }
}

// publish a message to a topic:
void mqttUpdate(String thisTopic, String message) {
  if (mqttClient.connected()) {
    // start a new message on the topic:
    mqttClient.beginMessage(thisTopic);
    // print the body of the message:
    mqttClient.print(message);
    // send the message:
    mqttClient.endMessage();
    // send a serial notification:
    Serial.print("published to: ");
    Serial.print(thisTopic);
    Serial.print(",  message: ");
    Serial.println(message);
    // timestamp this message:
    lastTimeSent = millis();
  }
}

boolean connectToBroker() {
  // if the MQTT client is not connected:
  if (!mqttClient.connect(broker, port)) {
    // print out the error message:
    Serial.print("MQTT connection failed. Error no: ");
    Serial.println(mqttClient.connectError());
    digitalWrite(MQTTPin, LOW);
    // return that you're not connected:
    return false;
  } else {
    digitalWrite(MQTTPin, HIGH);
  }

  // set the message receive callback:
  mqttClient.onMessage(onMqttMessage);
  // subscribe to a topic and all subtopics:
  Serial.print("Subscribing to topic: ");
  Serial.println(topic + "/#");
  mqttClient.subscribe(topic + "/#");

  // once you're connected, you
  // return that you're connected:
  return true;
}

void onMqttMessage(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.println("Received a message with topic ");
  Serial.print(mqttClient.messageTopic());
  Serial.print(", length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");
  // get the subtopic. Assumes the messageTopic is "topic/" then something:
  String subTopic = mqttClient.messageTopic().substring(topic.length() + 1);
  // an empty string to read the result
  String incoming = "";
  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    incoming += (char)mqttClient.read();
  }
  // print the incoming message:
  Serial.println(incoming);
}


void connectToNetwork() {
  // try to connect to the network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Attempting to connect to: " + String(SECRET_SSID));
    //Connect to WPA / WPA2 network:
    WiFi.begin(SECRET_SSID, SECRET_PASS);
    delay(2000);
  }
  // print IP address once connected:
  Serial.print("Connected. My IP address: ");
  Serial.println(WiFi.localIP());
}