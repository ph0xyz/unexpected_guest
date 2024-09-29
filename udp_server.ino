#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <WebSocketsServer_Generic.h>
#include <FS.h>
#include <ArduinoJson.h>

// Define the number of motors
#define NUM_MOTORS 11

// WiFi credentials
const char* ssid = "CeilingControlAP";
const char* password = "idrf2024";

// UDP settings
WiFiUDP udp;
const unsigned int udpPort = 12345; // Port for UDP communication
IPAddress broadcastIP(192, 168, 4, 255); // Broadcast IP for the AP network

// HTTP Server
ESP8266WebServer server(80);
WebSocketsServer webSocket(81); // WebSocket server on port 81

// Motor data structure
struct MotorData {
  uint8_t speed;
  uint8_t modes[NUM_MOTORS]; // 0: wave, 1: down-up, 2: constant
} motorData;

// HTML page with WebSocket support
const char* html = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Motor Control</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            min-height: 100vh;
            background-color: #f4f4f9;
        }
        .container {
            width: 100%;
            max-width: 400px;
            padding: 20px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            background-color: white;
            border-radius: 8px;
        }
        h2 {
            text-align: center;
            color: #333;
        }
        .motor-control {
            margin: 15px 0;
        }
        .button-group {
            display: flex;
            justify-content: space-between;
            margin-bottom: 10px;
        }
        .button-group button {
            flex: 1;
            padding: 8px;
            margin-right: 5px;
            border: none;
            color: white;
            font-size: 14px;
            cursor: pointer;
            border-radius: 4px;
        }
        .button-group button:last-child {
            margin-right: 0;
        }
        .wave-button {
            background-color: #007BFF;
        }
        .down-up-button {
            background-color: #FFC107;
        }
        .constant-button {
            background-color: #28A745;
        }
        input[type="range"] {
            width: 100%;
            margin: 15px 0;
        }
        label {
            display: block;
            font-size: 16px;
            color: #333;
            margin-bottom: 5px;
        }
        .selected {
            border: 2px solid #333;
            opacity: 1;
        }
        .unselected {
            opacity: 0.6;
        }
    </style>
</head>
<body>
    <div class="container">
        <h2>Motor Control Interface</h2>
        <label for="speed">Speed (0-100):</label>
        <input type="range" id="speed" name="speed" min="0" max="100" value="30" oninput="updateSpeedLabel(this.value)">
        <span id="speedValue">30</span>
        <div id="motor-controls"></div>
    </div>
    <script>
        const motors = 11;
        const motorControls = document.getElementById('motor-controls');
        const ws = new WebSocket(`ws://${location.hostname}:81/`); // WebSocket connection

        // Function to update speed label and send data via WebSocket
        function updateSpeedLabel(value) {
            document.getElementById('speedValue').innerText = value;
            ws.send(JSON.stringify({ action: 'setSpeed', speed: value }));
        }

        for (let i = 0; i < motors; i++) {
            const div = document.createElement('div');
            div.classList.add('motor-control');
            div.innerHTML = `
                <h3>Motor ${i + 1}</h3>
                <div class="button-group">
                    <button type="button" class="wave-button" onclick="selectMode(${i}, 0)">Wave</button>
                    <button type="button" class="down-up-button" onclick="selectMode(${i}, 1)">Down-Pause-Up</button>
                    <button type="button" class="constant-button" onclick="selectMode(${i}, 2)">Constant</button>
                </div>
                <input type="hidden" id="mode${i}" name="mode${i}" value="0">
            `;
            motorControls.appendChild(div);
        }

        function selectMode(motor, mode) {
            document.getElementById(`mode${motor}`).value = mode;
            const buttons = document.querySelectorAll(`.motor-control:nth-child(${motor + 1}) .button-group button`);
            buttons.forEach(button => button.classList.remove('selected'));
            buttons.forEach(button => button.classList.add('unselected'));
            buttons[mode].classList.add('selected');
            buttons[mode].classList.remove('unselected');

            ws.send(JSON.stringify({ action: 'setMode', motor: motor, mode: mode }));
        }

        // Initialize all buttons to have "wave" selected by default
        for (let i = 0; i < motors; i++) {
            selectMode(i, 0);
        }
    </script>
</body>
</html>
)=====";

// Function to handle WebSocket events
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_TEXT: {
      String message = String((char *)payload).substring(0, length);
      Serial.printf("Received WebSocket message: %s\n", message.c_str());

      // Parse incoming message
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, message);
      if (error) {
        Serial.print("Failed to parse WebSocket message: ");
        Serial.println(error.c_str());
        return;
      }

      const char* action = doc["action"];
      if (strcmp(action, "setSpeed") == 0) {
        motorData.speed = doc["speed"];
        Serial.printf("Speed updated to: %d\n", motorData.speed);
      } else if (strcmp(action, "setMode") == 0) {
        int motor = doc["motor"];
        int mode = doc["mode"];
        if (motor >= 0 && motor < NUM_MOTORS) {
          motorData.modes[motor] = mode;
          Serial.printf("Motor %d mode updated to: %d\n", motor + 1, mode);
        }
      }
      break;
    }
    default:
      break;
  }
}

void setup() {
  // Initialize Serial
  Serial.begin(115200);

  // Initialize motor data with default values
  motorData.speed = 30; // Default speed
  for (int i = 0; i < NUM_MOTORS; i++) {
    motorData.modes[i] = 0; // Default mode: wave
  }

  // Start WiFi in AP mode
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");

  // Initialize UDP
  udp.begin(udpPort);
  Serial.printf("UDP Server started on IP %s, port %d\n", WiFi.softAPIP().toString().c_str(), udpPort);

  // Define HTTP server routes
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", html);
  });

  // Start HTTP server
  server.begin();
  Serial.println("HTTP Server started");

  // Start WebSocket server
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  Serial.println("WebSocket Server started");
}

void loop() {
  // Handle HTTP client requests
  server.handleClient();

  // Handle WebSocket events
  webSocket.loop();

  // Broadcast motor data over UDP
  udp.beginPacket(broadcastIP, udpPort);
  udp.write((uint8_t*)&motorData, sizeof(MotorData));
  udp.endPacket();
  Serial.println("Broadcasting motor data...");

  delay(100); // Reduced broadcast delay for more real-time updates
}
