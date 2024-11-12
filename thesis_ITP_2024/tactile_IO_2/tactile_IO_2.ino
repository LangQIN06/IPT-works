#include <WiFiNINA.h>  // use this for Nano 33 IoT, MKR1010, Uno WiFi
#include <ArduinoMqttClient.h>
#include <Wire.h>
#include <Servo.h>
#include <ArduinoJson.h>  
#include "arduino_secrets.h"


// initialize WiFi connection as SSL:
//WiFiSSLClient wifi;
WiFiClient wifi;
MqttClient mqttClient(wifi);

// details for MQTT client:
char broker[] = "public.cloud.shiftr.io";
//char broker[] = "test.mosquitto.org";
int port = 1883;
String topic = "lq";
char clientID[] = "Client-lang";

// last time the client sent a message, in ms:
long lastTimeSent = 0;

String macAddr;

//define OUTPUT;
int LED_PIN = 3;
int NETWORK_LED_PIN = 4;  // network
int SERVO_PIN = 9;
int SERVO_PIN_2 = 10;
Servo myServo;
Servo myServo2;
int brightness = 5; 

//error
int buttonPin = 2;                    // button
volatile bool buttonPressed = false;  // volatile

// // ACS712设置
// const int analogInPin = A0;  // ACS712-A0
// float sensorValue = 0;
// float current = 0;
// unsigned long lastTime = 0;
// unsigned long interval = 100;  
// int lastSensorValue = 0;       // last sensorValue
// int threshold = 35;

void setup() {
  // initialize serial:
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(NETWORK_LED_PIN, OUTPUT);
  pinMode(buttonPin, INPUT);  // button
  myServo.attach(SERVO_PIN);
  myServo2.attach(SERVO_PIN_2);

  // servos original position
  myServo.write(0);     // servo1-0
  myServo2.write(180);  // servo2-180

  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButtonPress, FALLING);  

  // wait for serial monitor to open:
  if (!Serial) delay(3000);

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
}


void loop() {
  //if you disconnected from the network, reconnect:
  if (WiFi.status() != WL_CONNECTED) {
    connectToNetwork();
    // skip the rest of the loop until you are connected:
    return;
  }

  // if not connected to the broker, try to connect:
  if (!mqttClient.connected()) {
    Serial.println("attempting to connect to broker");
    connectToBroker();
  }

  // poll for new messages from the broker:
  mqttClient.poll();


  if (buttonPressed) {
    // press button
    sendButtonPressMessage();
    buttonPressed = false;  // reset
  }

  // // check current and read
  // checkAndPrintCurrent();
}

void handleButtonPress() {
  buttonPressed = true;
  Serial.println(1);
}

void sendButtonPressMessage() {
  DynamicJsonDocument doc(256);
  doc["error"] = 1;  // when pressed button, send error 1
  String output;
  serializeJson(doc, output);

  // 发送MQTT消息
  if (mqttClient.connected()) {
    mqttClient.beginMessage(topic + "/device/error");
    mqttClient.print(output);
    mqttClient.endMessage();
  }
}

void blinkLED() {
  digitalWrite(LED_PIN, brightness);  // LED
  delay(500);                   
  digitalWrite(LED_PIN, LOW);   
  delay(500);                   
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
    // return that you're not connected:
    return false;
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
  while (mqttClient.available()) {
    incoming += (char)mqttClient.read();
  }

  Serial.print("Received message: ");
  Serial.println(incoming);


  DynamicJsonDocument doc(4096);  // JSON size
  deserializeJson(doc, incoming);

  // check loop
  if (doc.is<JsonArray>() && doc[0].is<JsonArray>()) {
    // loop
    JsonArray actionGroups = doc.as<JsonArray>();
    for (JsonArray actionGroup : actionGroups) {
      // for (JsonObject action : actionGroup) {

        JsonObject servo1 = actionGroup[0];
        JsonObject servo2 = actionGroup[1];

        int servoId1 = servo1["servoId"];
        int startAngle1 = servo1["startAngle"];
        int endAngle1 = servo1["endAngle"];
        int speed1 = servo1["speed"];
        int times1 = servo1["times"];
        int directionInterval1 = servo1["directionInterval"];
        int loopInterval1 = servo1["loopInterval"];

        int servoId2 = servo2["servoId"];
        int startAngle2 = servo2["startAngle"];
        int endAngle2 = servo2["endAngle"];
        int speed2 = servo2["speed"];
        int times2 = servo2["times"];
        int directionInterval2 = servo2["directionInterval"];
        int loopInterval2 = servo1["loopInterval"];

        // control TWO servos
        controlServos(servoId1, startAngle1, endAngle1, speed1, times1, directionInterval1, loopInterval1,
                      servoId2, startAngle2, endAngle2, speed2, times2, directionInterval2, loopInterval2);
      // }
      delay(500);
    }
  } else if (doc.is<JsonArray>()) {
    // single array
    JsonArray actionArray = doc.as<JsonArray>();
    for (JsonObject action : actionArray) {
      int servoId = action["servoId"];
      int startAngle = action["startAngle"];
      int endAngle = action["endAngle"];
      int speed = action["speed"];
      int times = action["times"];
      int directionInterval = action["directionInterval"];
      int loopInterval = action["loopInterval"];

      // call function of single servo
      controlSingleServo(servoId, startAngle, endAngle, speed, times, directionInterval, loopInterval);
      delay(500);
    }
  }
}

// Function to control two servos (synchronous movement, add speed control, retain round-trip movement)
void controlServos(int id1, int start1, int end1, int speed1, int times1, int directionInterval1, int loopInterval1,
                   int id2, int start2, int end2, int speed2, int times2, int directionInterval2, int loopInterval2) {
  // blink after sending command
  blinkLED();

  Servo &servo1 = (id1 == 0) ? myServo : myServo2;
  Servo &servo2 = (id2 == 0) ? myServo : myServo2;



  if (times1 <= times2) {
    for (int i = 0; i < max(times1, times2); i++) {
      if (i < times1) {
        moveServoSync(servo1, servo2, start1, end1, speed1, start2, end2, speed2);
        delay(directionInterval1);  // direction interval
        moveServoSync(servo1, servo2, end1, start1, speed1, end2, start2, speed2);
      } else if (i < times2) {
        moveServoSync(servo1, servo2, start1, start1, speed1, start2, end2, speed2);
        delay(directionInterval2);  // direction interval
        moveServoSync(servo1, servo2, start1, start1, speed1, end2, start2, speed2);
      }

      if (i < max(times1, times2) - 1) {
        delay(max(loopInterval1, loopInterval2));  // loop interval
      }
    }
  }

  if (times1 > times2) {
    for (int j = 0; j < max(times1, times2); j++) {
      if (j < times2) {
        moveServoSync(servo1, servo2, start1, end1, speed1, start2, end2, speed2);
        delay(directionInterval1);  // direction interval
        moveServoSync(servo1, servo2, end1, start1, speed1, end2, start2, speed2);
      } else if (j < times1) {
        moveServoSync(servo1, servo2, start1, start1, speed1, start2, end2, speed2);
        delay(directionInterval2);  // direction interval
        moveServoSync(servo1, servo2, start1, start1, speed1, end2, start2, speed2);
      }

      if (j < max(times1, times2) - 1) {
        delay(max(loopInterval1, loopInterval2));  // loop interval
      }
    }
  }
}

// auxiliary function for moving two servos synchronously
void moveServoSync(Servo &servo1, Servo &servo2, int startAngle1, int endAngle1, int speed1,
                   int startAngle2, int endAngle2, int speed2) {
  int step1 = (endAngle1 > startAngle1) ? 1 : -1;
  int step2 = (endAngle2 > startAngle2) ? 1 : -1;
  int pos1 = startAngle1, pos2 = startAngle2;


  while (pos1 != endAngle1 || pos2 != endAngle2) {
    if (pos1 != endAngle1) {
      servo1.write(pos1);
      pos1 += step1;
    }
    if (pos2 != endAngle2) {
      servo2.write(pos2);
      pos2 += step2;
    }
    delay(min(1000 / speed1, 1000 / speed2));  
  }
  servo1.write(endAngle1);
  servo2.write(endAngle2);
}


//  control a single servo motor (default round trip, with speed control)
void controlSingleServo(int servoId, int startAngle, int endAngle, int speed, int times, int directionInterval, int loopInterval) {

  // blink after sending command
  blinkLED();

  Servo &selectedServo = (servoId == 0) ? myServo : myServo2;


  for (int i = 0; i < times; i++) {
    moveServo(selectedServo, startAngle, endAngle, speed);
    delay(directionInterval);  // delay between opposite directions
    moveServo(selectedServo, endAngle, startAngle, speed);

    if (i < times - 1) {
      delay(loopInterval);  // delay before next loop
    }
  }
}


// Auxiliary functions for servos (with speed control)
void moveServo(Servo &servo, int startAngle, int endAngle, int speed) {
  int step = (endAngle > startAngle) ? 1 : -1;
  for (int pos = startAngle; pos != endAngle; pos += step) {
    servo.write(pos);
    delay(1000 / speed);  // speed control
  }
  servo.write(endAngle);  // ensure stop at destination
}


void connectToNetwork() {
  unsigned long blinkTime = millis();
  bool ledState = false;

  Serial.println("Attempting to connect to network...");

  while (WiFi.status() != WL_CONNECTED) {
    // blink
    if (millis() - blinkTime > 100) {
      blinkTime = millis();
      ledState = !ledState;
      digitalWrite(NETWORK_LED_PIN, ledState ? HIGH : LOW);
    }

    // connect to wifi
    if (WiFi.begin(SECRET_SSID, SECRET_PASS) == WL_CONNECTED) {
      break;  
    }

    delay(100);  
  }

  // LED turn on if successed
  digitalWrite(NETWORK_LED_PIN, HIGH);
  Serial.print("Connected. My IP address: ");
  Serial.println(WiFi.localIP());
}
