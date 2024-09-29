#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>

// WiFi credentials
const char* ssid = "Your_SSID";   // Replace with your WiFi SSID
const char* password = "Your_PASSWORD"; // Replace with your WiFi Password

// Server and UDP settings
ESP8266WebServer server(80);      // HTTP server running on port 80
WiFiUDP udp;                      // UDP object for sending commands
const unsigned int udpPort = 12345; // UDP port for sending commands

// Motor clients' IP addresses and their respective IDs
IPAddress motorIPs[11] = {
  IPAddress(192, 168, 1, 101),  // Motor 1
  IPAddress(192, 168, 1, 102),  // Motor 2
  IPAddress(192, 168, 1, 103),  // Motor 3
  IPAddress(192, 168, 1, 104),  // Motor 4
  IPAddress(192, 168, 1, 105),  // Motor 5
  IPAddress(192, 168, 1, 106),  // Motor 6
  IPAddress(192, 168, 1, 107),  // Motor 7
  IPAddress(192, 168, 1, 108),  // Motor 8
  IPAddress(192, 168, 1, 109),  // Motor 9
  IPAddress(192, 168, 1, 110),  // Motor 10
  IPAddress(192, 168, 1, 111)   // Motor 11
};

// Function to send UDP command
void sendUDPCommand(IPAddress ip, const char* message) {
  udp.beginPacket(ip, udpPort);
  udp.write(message);
  udp.endPacket();
}

// Function to handle start all motors
void handleStartAll() {
  for (int i = 0; i < 11; i++) {
    sendUDPCommand(motorIPs[i], "start0");
  }
  server.send(200, "text/html", "Started all motors");
}

// Function to handle stop all motors
void handleStopAll() {
  for (int i = 0; i < 11; i++) {
    sendUDPCommand(motorIPs[i], "stop0");
  }
  server.send(200, "text/html", "Stopped all motors");
}

// Function to handle wave command (start motors sequentially with a 10s delay)
void handleWave() {
  for (int i = 0; i < 11; i++) {
    sendUDPCommand(motorIPs[i], String("start" + String(i + 1)).c_str());
    delay(10000); // 10-second delay between starting each motor
  }
  server.send(200, "text/html", "Wave command sent");
}

// Functions to handle individual motor start and stop
void handleMotorStart(int motorId) {
  if (motorId >= 1 && motorId <= 11) {
    sendUDPCommand(motorIPs[motorId - 1], String("start" + String(motorId)).c_str());
    server.send(200, "text/html", "Started motor " + String(motorId));
  }
}

void handleMotorStop(int motorId) {
  if (motorId >= 1 && motorId <= 11) {
    sendUDPCommand(motorIPs[motorId - 1], String("stop" + String(motorId)).c_str());
    server.send(200, "text/html", "Stopped motor " + String(motorId));
  }
}

// HTML page for motor control
const char htmlPage[] PROGMEM = R"====(
<!DOCTYPE html>
<html>
  <head>
    <title>Motor Control</title>
  </head>
  <body>
    <h1>Motor Control</h1>
    <button onclick="sendCommand('startall')">Start All</button>
    <button onclick="sendCommand('stopall')">Stop All</button>
    <button onclick="sendCommand('wave')">Wave</button>
    <hr />
    <h2>Individual Motor Control</h2>
    <div>
      <button onclick="sendCommand('start1')">Motor 1 ON</button>
      <button onclick="sendCommand('stop1')">Motor 1 OFF</button><br>
      <button onclick="sendCommand('start2')">Motor 2 ON</button>
      <button onclick="sendCommand('stop2')">Motor 2 OFF</button><br>
      <button onclick="sendCommand('start3')">Motor 3 ON</button>
      <button onclick="sendCommand('stop3')">Motor 3 OFF</button><br>
      <button onclick="sendCommand('start4')">Motor 4 ON</button>
      <button onclick="sendCommand('stop4')">Motor 4 OFF</button><br>
      <button onclick="sendCommand('start5')">Motor 5 ON</button>
      <button onclick="sendCommand('stop5')">Motor 5 OFF</button><br>
      <button onclick="sendCommand('start6')">Motor 6 ON</button>
      <button onclick="sendCommand('stop6')">Motor 6 OFF</button><br>
      <button onclick="sendCommand('start7')">Motor 7 ON</button>
      <button onclick="sendCommand('stop7')">Motor 7 OFF</button><br>
      <button onclick="sendCommand('start8')">Motor 8 ON</button>
      <button onclick="sendCommand('stop8')">Motor 8 OFF</button><br>
      <button onclick="sendCommand('start9')">Motor 9 ON</button>
      <button onclick="sendCommand('stop9')">Motor 9 OFF</button><br>
      <button onclick="sendCommand('start10')">Motor 10 ON</button>
      <button onclick="sendCommand('stop10')">Motor 10 OFF</button><br>
      <button onclick="sendCommand('start11')">Motor 11 ON</button>
      <button onclick="sendCommand('stop11')">Motor 11 OFF</button><br>
    </div>
    <script>
      function sendCommand(command) {
        fetch('/' + command)
          .then(response => response.text())
          .then(data => alert(data));
      }
    </script>
  </body>
</html>
)====";

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Initialize UDP
  udp.begin(udpPort);

  // Setup HTTP server routes
  server.on("/", []() {
    server.send_P(200, "text/html", htmlPage);
  });
  
  server.on("/startall", handleStartAll);
  server.on("/stopall", handleStopAll);
  server.on("/wave", handleWave);

  // Setup routes for individual motor control
  for (int i = 1; i <= 11; i++) {
    server.on("/start" + String(i), [i]() { handleMotorStart(i); });
    server.on("/stop" + String(i), [i]() { handleMotorStop(i); });
  }

  // Start the HTTP server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle HTTP requests
  server.handleClient();
}
