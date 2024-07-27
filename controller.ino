#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>

const char* ssid = "ceilc2";
const char* password = "PASSWORD_HERE";

ESP8266WebServer server(80);
WiFiUDP udp;
unsigned int localUdpPort = 4210;

String heartbeatStatus[11];
unsigned long lastHeartbeat[11];

void setupWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
}

void setupServer() {
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();
  Serial.println("HTTP server started");
}

void handleRoot() {
  String html = "<html><body>";
  for (int i = 1; i <= 11; i++) {
    html += "<h3>Motor " + String(i) + " - " + heartbeatStatus[i-1] + "</h3>";
    html += "Speed: <input type='range' min='0' max='100' value='50' id='speed" + String(i) + "'><br>";
    html += "Angle: <input type='range' min='0' max='360' value='180' id='angle" + String(i) + "'><br>";
    html += "<button onclick='sendSettings(" + String(i) + ")'>Set</button><br>";
  }
  html += "<script>";
  html += "function sendSettings(motor) {";
  html += "var speed = document.getElementById('speed' + motor).value;";
  html += "var angle = document.getElementById('angle' + motor).value;";
  html += "var xhttp = new XMLHttpRequest();";
  html += "xhttp.open('GET', '/set?motor=' + motor + '&speed=' + speed + '&angle=' + angle, true);";
  html += "xhttp.send();";
  html += "}";
  html += "</script>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleSet() {
  String motor = server.arg("motor");
  String speed = server.arg("speed");
  String angle = server.arg("angle");

  String ip = "192.168.1." + motor;
  String message = "speed=" + speed + "&angle=" + angle;
  udp.beginPacket(ip.c_str(), localUdpPort);
  udp.write(message.c_str());
  udp.endPacket();

  server.send(200, "text/plain", "Settings sent to motor " + motor);
}

void checkHeartbeat() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char incomingPacket[255];
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
    }

    String packet = String(incomingPacket);
    if (packet.startsWith("heartbeat")) {
      int motor = packet.substring(9).toInt();
      heartbeatStatus[motor - 1] = "Connected";
      lastHeartbeat[motor - 1] = millis();
    }
  }

  for (int i = 0; i < 11; i++) {
    if (millis() - lastHeartbeat[i] > 5000) {
      heartbeatStatus[i] = "Disconnected";
    }
  }
}

void setup() {
  Serial.begin(115200);
  setupWiFi();
  udp.begin(localUdpPort);
  setupServer();

  for (int i = 0; i < 11; i++) {
    heartbeatStatus[i] = "Disconnected";
    lastHeartbeat[i] = 0;
  }
}

void loop() {
  server.handleClient();
  checkHeartbeat();
}
