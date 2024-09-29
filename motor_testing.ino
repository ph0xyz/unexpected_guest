#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <AccelStepper.h>

// Define motor interface type
#define MotorInterfaceType 1

// Define pins for the stepper motor and end switches
#define PUL_PIN D0               // Pulse pin for the stepper motor
#define DIR_PIN D1               // Direction pin for the stepper motor
#define ENA_PIN D2               // Enable pin for the stepper motor driver
#define END_SWITCH_1 D7          // End-switch 1
#define END_SWITCH_2 D6          // End-switch 2
#define BLINK_LED_PIN LED_BUILTIN // On-board LED

// Define the motor ID for this client (1-indexed)
const int motorId = 3; // Set this to the motor ID of this specific motor

// WiFi credentials
const char* ssid = "CeilingControlAP";
const char* password = "idrf2024";

// UDP settings
WiFiUDP udp;
const unsigned int udpPort = 12345; // Port for UDP communication

// Motor data structure
struct MotorData {
  uint8_t speed;     // Speed range 0-100
  uint8_t modes[11]; // Mode array for different motors
} motorData;

// Create an instance of AccelStepper
AccelStepper stepper(MotorInterfaceType, PUL_PIN, DIR_PIN);

// Variables for LED blinking and motor control
bool buttonState1 = HIGH;     // Current state of END_SWITCH_1
bool lastButtonState1 = HIGH; // Previous state of END_SWITCH_1
bool actionTaken1 = false;    // Flag to check if action is taken for END_SWITCH_1

bool buttonState2 = HIGH;     // Current state of END_SWITCH_2
bool lastButtonState2 = HIGH; // Previous state of END_SWITCH_2
bool actionTaken2 = false;    // Flag to check if action is taken for END_SWITCH_2

unsigned long lastDebounceTime1 = 0; // Time the button state was last changed for END_SWITCH_1
unsigned long lastDebounceTime2 = 0; // Time the button state was last changed for END_SWITCH_2

const long debounceDelay = 50; // Debounce delay in milliseconds

long speed = 500; // Initial motor speed
bool isDirectionPositive = true; // Direction flag

// Variables for managing UDP reception
unsigned long lastPacketTime = 0;  // Timestamp of the last received packet
const unsigned long timeoutDuration = 2000;  // Timeout duration in milliseconds

// Default speed when no UDP packets are received
const int defaultSpeed = 3000;

// Variables for handling the motor limits
long leftLimit = 0;
long rightLimit = 0;
long centerPosition = 0;
long stepsPerDegree = 10;  // Example: 10 steps per degree, adjust based on your stepper

// Function to map speed from 0-100 to 300-1000
int mapSpeed(int inputSpeed) {
  return map(inputSpeed, 0, 100, 100, 3000);
}

void setup() {
  // Initialize Serial
  Serial.begin(115200);

  // Initialize pins
  pinMode(ENA_PIN, OUTPUT);
  pinMode(BLINK_LED_PIN, OUTPUT);
  pinMode(END_SWITCH_1, INPUT_PULLUP); // Using INPUT_PULLUP to ensure normally open
  pinMode(END_SWITCH_2, INPUT_PULLUP); // Using INPUT_PULLUP to ensure normally open

  digitalWrite(ENA_PIN, LOW); // Enable the driver (active LOW)

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
  Serial.println("UDP Client started");

  // Initialize the stepper motor
  stepper.setMaxSpeed(1000);    // Set maximum speed
  stepper.setAcceleration(500); // Set acceleration

  // Find end switches and center the motor
  findLimitsAndCenter();
}

void loop() {
  // Check for UDP commands
  checkUDPCommands();

  // Run the stepper motor based on the current command
  stepper.runSpeed();
}

void findLimitsAndCenter() {
  // Move left towards END_SWITCH_1
  stepper.setSpeed(-500); // Move left
  while (digitalRead(END_SWITCH_1) != LOW) {
    stepper.runSpeed();
  }
  leftLimit = stepper.currentPosition(); // Set left limit

  // Move right towards END_SWITCH_2
  stepper.setSpeed(500); // Move right
  while (digitalRead(END_SWITCH_2) != LOW) {
    stepper.runSpeed();
  }
  rightLimit = stepper.currentPosition(); // Set right limit

  // Calculate center position
  centerPosition = (leftLimit + rightLimit) / 2;

  // Move to center
  stepper.moveTo(centerPosition);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
  Serial.println("Motor centered");
}

void checkUDPCommands() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char incomingPacket[255];
    udp.read(incomingPacket, 255);
    incomingPacket[packetSize] = '\0';

    String command = String(incomingPacket);

    // Check if the command is for all motors (e.g., "start0" or "stop0")
    if (command.startsWith("start0")) {
      // Start all motors
      startMotorMovement();
    } else if (command.startsWith("stop0")) {
      // Stop all motors and center them
      stopMotorAndCenter();
    }
    // Check if the command is for the current motor ID (e.g., "start3" or "stop3")
    else if (command == "start" + String(motorId)) {
      // Start movement between limits with 5-degree buffer
      startMotorMovement();
    } else if (command == "stop" + String(motorId)) {
      // Stop and move to center
      stopMotorAndCenter();
    }
  }
}

void startMotorMovement() {
  Serial.println("Start command received, moving between limits");

  // Move towards END_SWITCH_1 with 5-degree buffer
  long bufferSteps = stepsPerDegree * 5;
  stepper.moveTo(leftLimit + bufferSteps);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }

  // Now move towards END_SWITCH_2 with 5-degree buffer
  stepper.moveTo(rightLimit - bufferSteps);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }

  // Continue looping between the limits (non-blocking)
  stepper.setSpeed(500); // Set speed for continuous movement
}

void stopMotorAndCenter() {
  Serial.println("Stop command received, returning to center");

  // Stop current movement and move to center
  stepper.moveTo(centerPosition);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
  Serial.println("Motor centered");
}
