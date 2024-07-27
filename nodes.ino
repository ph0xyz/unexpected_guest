#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <AccelStepper.h>

// WiFi credentials
const char* ssid = "ceilc2";
const char* password = "idrf2024";

// Static IP address configuration
IPAddress local_IP(192, 168, 1, 2); // Change this for each slave (192.168.1.2, 192.168.1.3, etc.)
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Master IP address
IPAddress masterIP(192, 168, 1, 99);

ESP8266WebServer server(80);
WiFiUDP udp;
unsigned int localUdpPort = 4210;

#define MOTOR_PIN1 D1
#define MOTOR_PIN2 D2
#define MOTOR_PIN3 D3
#define MOTOR_PIN4 D4

#define END_SWITCH1_PIN D5
#define END_SWITCH2_PIN D6

AccelStepper stepper(AccelStepper::FULL4WIRE, MOTOR_PIN1, MOTOR_PIN2, MOTOR_PIN3, MOTOR_PIN4);

int maxAngle = 180;
int speed = 50;
int zeroPosition = 0;

unsigned long lastHeartbeat = 0;

void setupWiFi() {
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
}

void setupServer() {
  server.on("/update", handleUpdate);
  server.begin();
  Serial.println("HTTP server started");
}

void calibrateZero() {
  stepper.setSpeed(100);
  while (digitalRead(END_SWITCH1_PIN) != LOW) {
    stepper.runSpeed();
  }
  stepper.stop();

  stepper.setSpeed(-100);
  while (digitalRead(END_SWITCH2_PIN) != LOW) {
    stepper.runSpeed();
  }
  stepper.stop();

  zeroPosition = (stepper.currentPosition() / 2);
  stepper.setCurrentPosition(0);
  stepper.moveTo(zeroPosition);
  stepper.runToPosition();
}

void handleUpdate() {
  speed = server.arg("speed").toInt();
  maxAngle = server.arg("angle").toInt();
  server.send(200, "text/plain", "Update received");
}

void sendHeartbeat() {
  String message = "heartbeat" + String(WiFi.localIP()[3]);
  udp.beginPacket(masterIP, localUdpPort);
  udp.write(message.c_str());
  udp.endPacket();
}

void checkIncomingPacket() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char incomingPacket[255];
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
    }

    String packet = String(incomingPacket);
    if (packet.startsWith("speed=")) {
      speed = packet.substring(6, packet.indexOf('&')).toInt();
      maxAngle = packet.substring(packet.indexOf('=') + 1).toInt();
    }
  }
}

void moveToTargetPosition(int targetPosition) {
  stepper.setMaxSpeed(speed);
  stepper.setAcceleration(50);
  stepper.moveTo(targetPosition);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
    if (digitalRead(END_SWITCH1_PIN) == LOW || digitalRead(END_SWITCH2_PIN) == LOW) {
      stepper.setSpeed(-stepper.speed());
    }
  }
}

void loop() {
  server.handleClient();
  checkIncomingPacket();

  int targetPosition = zeroPosition + maxAngle;
  moveToTargetPosition(targetPosition);
  delay(1000);

  moveToTargetPosition(zeroPosition - maxAngle);
  delay(1000);

  if (millis() - lastHeartbeat > 1000) {
    sendHeartbeat();
    lastHeartbeat = millis();
  }
}

void setup() {
  Serial.begin(115200);
  setupWiFi();
  udp.begin(localUdpPort);
  setupServer();

  pinMode(END_SWITCH1_PIN, INPUT_PULLUP);
  pinMode(END_SWITCH2_PIN, INPUT_PULLUP);

  calibrateZero();
}
