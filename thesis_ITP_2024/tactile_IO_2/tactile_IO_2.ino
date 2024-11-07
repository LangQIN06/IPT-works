#include <WiFiNINA.h>  // use this for Nano 33 IoT, MKR1010, Uno WiFi
#include <ArduinoMqttClient.h>
#include <Wire.h>
#include <Servo.h>
#include <ArduinoJson.h>  // 引入ArduinoJson库
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
int NETWORK_LED_PIN = 4;  // 定义网络状态指示LED的引脚为4
int SERVO_PIN = 9;
int SERVO_PIN_2 = 10;
Servo myServo;
Servo myServo2;
int brightness = 5; 

//error
int buttonPin = 2;                    // 假设按钮连接到数字脚3
volatile bool buttonPressed = false;  // 使用volatile关键字，因为这个变量会在中断服务程序中被修改

// // ACS712设置
// const int analogInPin = A0;  // ACS712连接到A0
// float sensorValue = 0;
// float current = 0;
// unsigned long lastTime = 0;
// unsigned long interval = 100;  // 检测间隔，300毫秒/200ms
// int lastSensorValue = 0;       // 存储上一次的sensorValue
// int threshold = 35;

void setup() {
  // initialize serial:
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(NETWORK_LED_PIN, OUTPUT);
  pinMode(buttonPin, INPUT);  // button
  myServo.attach(SERVO_PIN);
  myServo2.attach(SERVO_PIN_2);

  // 设置servos初始位置
  myServo.write(0);     // servo1设置为0度
  myServo2.write(180);  // servo2设置为180度

  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButtonPress, FALLING);  // 假设按钮使用上升沿触发

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
    // 执行按钮按下后的操作
    sendButtonPressMessage();
    buttonPressed = false;  // 重置标志
  }

  // // 检查是否需要读取并打印电流
  // checkAndPrintCurrent();
}

void handleButtonPress() {
  buttonPressed = true;
  Serial.println(1);
}

void sendButtonPressMessage() {
  DynamicJsonDocument doc(256);
  doc["error"] = 1;  // 假设当按钮被按下时，我们发送一个错误状态"1"
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
  digitalWrite(LED_PIN, brightness);  // 打开LED
  delay(500);                   // 保持100毫秒
  digitalWrite(LED_PIN, LOW);   // 关闭LED
  delay(500);                   // 等待100毫秒
}


// void checkAndPrintCurrent() {
//   unsigned long currentTime = millis();
//   if (currentTime - lastTime >= interval) {
//     readAndPrintCurrent();
//     lastTime = currentTime;
//   }
// }

// void readAndPrintCurrent() {
//   sensorValue = analogRead(analogInPin);
//   int currentState = (abs(sensorValue - lastSensorValue) > threshold) ? 1 : 0;

//   // // 打印到串口监视器
//   // Serial.print("Sensor Value: ");
//   // Serial.println(sensorValue);
//   // Serial.println(currentState ? "1" : "0");

//   // 仅当currentState为1时，准备并发送MQTT消息
//   if (currentState == 1) {
//     // 准备MQTT消息
//     DynamicJsonDocument doc(256);
//     doc["sensorValue"] = sensorValue;
//     doc["currentState"] = currentState;
//     String output;
//     serializeJson(doc, output);

//     // 发送MQTT消息
//     mqttUpdate(topic + "/servo1/status", output);
//   }

//   lastSensorValue = sensorValue;  // 更新sensorValue
// }


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

  // // 处理消息
  // if (incoming == "LED OFF") {
  //   analogWrite(LED_PIN, 0);  // 关闭LED
  // } else if (incoming.startsWith("LED BRIGHTNESS ")) {
  //   int brightness = incoming.substring(15).toInt();
  //   analogWrite(LED_PIN, brightness);  // 调整LED亮度
  // }

  DynamicJsonDocument doc(4096);  // 增加JSON文档大小以容纳更大的消息
  deserializeJson(doc, incoming);

  // // 根据消息类型处理消息
  // // if (mqttClient.messageTopic() == topic + "/settings/update") {
  // // 检查并更新 threshold 和 interval
  // if (doc.containsKey("threshold")) {
  //   threshold = doc["threshold"].as<int>();
  //   Serial.print("Updated threshold: ");
  //   Serial.println(threshold);
  // }
  // if (doc.containsKey("interval")) {
  //   interval = doc["interval"].as<unsigned long>();
  //   Serial.print("Updated interval: ");
  //   Serial.println(interval);
  // }
  // // }

  // 检查是否为嵌套数组
  if (doc.is<JsonArray>() && doc[0].is<JsonArray>()) {
    // 处理嵌套数组
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

        // 控制两个伺服电机
        controlServos(servoId1, startAngle1, endAngle1, speed1, times1, directionInterval1, loopInterval1,
                      servoId2, startAngle2, endAngle2, speed2, times2, directionInterval2, loopInterval2);
      // }
      delay(500);
    }
  } else if (doc.is<JsonArray>()) {
    // 处理单层数组
    JsonArray actionArray = doc.as<JsonArray>();
    for (JsonObject action : actionArray) {
      int servoId = action["servoId"];
      int startAngle = action["startAngle"];
      int endAngle = action["endAngle"];
      int speed = action["speed"];
      int times = action["times"];
      int directionInterval = action["directionInterval"];
      int loopInterval = action["loopInterval"];

      // 调用控制单个伺服电机的函数
      controlSingleServo(servoId, startAngle, endAngle, speed, times, directionInterval, loopInterval);
      delay(500);
    }
  }
}

// 控制两个伺服电机的函数（同步移动，添加速度控制，保留往返移动）
void controlServos(int id1, int start1, int end1, int speed1, int times1, int directionInterval1, int loopInterval1,
                   int id2, int start2, int end2, int speed2, int times2, int directionInterval2, int loopInterval2) {
  // 在发送指令前后闪烁LED
  blinkLED();

  Servo &servo1 = (id1 == 0) ? myServo : myServo2;
  Servo &servo2 = (id2 == 0) ? myServo : myServo2;



  if (times1 <= times2) {
    for (int i = 0; i < max(times1, times2); i++) {
      if (i < times1) {
        moveServoSync(servo1, servo2, start1, end1, speed1, start2, end2, speed2);
        delay(directionInterval1);  // 方向间隔
        moveServoSync(servo1, servo2, end1, start1, speed1, end2, start2, speed2);
      } else if (i < times2) {
        moveServoSync(servo1, servo2, start1, start1, speed1, start2, end2, speed2);
        delay(directionInterval2);  // 方向间隔
        moveServoSync(servo1, servo2, start1, start1, speed1, end2, start2, speed2);
      }

      if (i < max(times1, times2) - 1) {
        delay(max(loopInterval1, loopInterval2));  // 循环间隔
      }
    }
  }

  if (times1 > times2) {
    for (int j = 0; j < max(times1, times2); j++) {
      if (j < times2) {
        moveServoSync(servo1, servo2, start1, end1, speed1, start2, end2, speed2);
        delay(directionInterval1);  // 方向间隔
        moveServoSync(servo1, servo2, end1, start1, speed1, end2, start2, speed2);
      } else if (j < times1) {
        moveServoSync(servo1, servo2, start1, start1, speed1, start2, end2, speed2);
        delay(directionInterval2);  // 方向间隔
        moveServoSync(servo1, servo2, start1, start1, speed1, end2, start2, speed2);
      }

      if (j < max(times1, times2) - 1) {
        delay(max(loopInterval1, loopInterval2));  // 循环间隔
      }
    }
  }
}

// 同步移动两个伺服电机的辅助函数
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
    delay(min(1000 / speed1, 1000 / speed2));  // 根据两个伺服电机的速度决定延迟
  }
  servo1.write(endAngle1);
  servo2.write(endAngle2);
}


// 控制单个伺服电机的函数（默认往返移动，带速度控制）
void controlSingleServo(int servoId, int startAngle, int endAngle, int speed, int times, int directionInterval, int loopInterval) {

  // 在发送指令前后闪烁LED
  blinkLED();

  Servo &selectedServo = (servoId == 0) ? myServo : myServo2;


  for (int i = 0; i < times; i++) {
    moveServo(selectedServo, startAngle, endAngle, speed);
    delay(directionInterval);  // 在两个方向之间的延迟
    moveServo(selectedServo, endAngle, startAngle, speed);

    if (i < times - 1) {
      delay(loopInterval);  // 在下一次循环之前的延迟
    }
  }
}


// 移动伺服电机的辅助函数（带速度控制）
void moveServo(Servo &servo, int startAngle, int endAngle, int speed) {
  int step = (endAngle > startAngle) ? 1 : -1;
  for (int pos = startAngle; pos != endAngle; pos += step) {
    servo.write(pos);
    delay(1000 / speed);  // 速度控制
  }
  servo.write(endAngle);  // 确保到达终点
}


void connectToNetwork() {
  unsigned long blinkTime = millis();
  bool ledState = false;

  Serial.println("Attempting to connect to network...");

  while (WiFi.status() != WL_CONNECTED) {
    // 每隔500毫秒切换LED状态，以实现闪烁效果
    if (millis() - blinkTime > 100) {
      blinkTime = millis();
      ledState = !ledState;
      digitalWrite(NETWORK_LED_PIN, ledState ? HIGH : LOW);
    }

    // 尝试连接到WiFi网络
    if (WiFi.begin(SECRET_SSID, SECRET_PASS) == WL_CONNECTED) {
      break;  // 如果连接成功，则退出循环
    }

    delay(100);  // 稍微等待，避免过于频繁的连接尝试
  }

  // 一旦连接成功，将LED设置为常亮
  digitalWrite(NETWORK_LED_PIN, HIGH);
  Serial.print("Connected. My IP address: ");
  Serial.println(WiFi.localIP());
}
