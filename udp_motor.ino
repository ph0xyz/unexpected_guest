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
const int motorId = 3; // Use motor ID 1

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
  //while (WiFi.status() != WL_CONNECTED) {
   // delay(500);
   // Serial.print(".");
  //}
  Serial.println("\nConnected to WiFi");

  // Initialize UDP
  udp.begin(udpPort);
  Serial.println("UDP Client started");

  // Initialize the stepper motor
  stepper.setMaxSpeed(1000);    // Set maximum speed
  stepper.setAcceleration(500); // Set acceleration
  speed = defaultSpeed;         // Initial speed
  stepper.setSpeed(speed);      // Set initial speed
}

void loop() {
  unsigned long currentMillis = millis();

  // Handle END_SWITCH_1
  int reading1 = digitalRead(END_SWITCH_1);
  if (reading1 != lastButtonState1) {
    lastDebounceTime1 = millis(); // Reset debounce timer
  }

  if ((millis() - lastDebounceTime1) > debounceDelay) {
    // If the button state has changed and is now LOW (button pressed)
    if (reading1 != buttonState1) {
      buttonState1 = reading1;

      // If the button is pressed (LOW) and not blocked, perform the action
      if (buttonState1 == LOW && !actionTaken1) {
        performAction1();
        actionTaken1 = true;
      }
    }
  }

  // If the button is released (HIGH), reset the action flag
  if (buttonState1 == HIGH) {
    actionTaken1 = false;
  }

  lastButtonState1 = reading1;

  // Handle END_SWITCH_2
  int reading2 = digitalRead(END_SWITCH_2);
  if (reading2 != lastButtonState2) {
    lastDebounceTime2 = millis(); // Reset debounce timer
  }

  if ((millis() - lastDebounceTime2) > debounceDelay) {
    // If the button state has changed and is now LOW (button pressed)
    if (reading2 != buttonState2) {
      buttonState2 = reading2;

      // If the button is pressed (LOW) and not blocked, perform the action
      if (buttonState2 == LOW && !actionTaken2) {
        performAction2();
        actionTaken2 = true;
      }
    }
  }

  // If the button is released (HIGH), reset the action flag
  if (buttonState2 == HIGH) {
    actionTaken2 = false;
  }

  lastButtonState2 = reading2;

  // Check for incoming UDP packets
  int packetSize = udp.parsePacket();
  if (packetSize) {
    // Read the UDP packet into the motorData struct
    int len = udp.read((uint8_t*)&motorData, sizeof(MotorData));

    // Debug: Print raw packet data
    Serial.print("Raw Packet Data Length: ");
    Serial.println(len);
    Serial.print("Raw Packet Data: ");
    for (int i = 0; i < len; i++) {
      Serial.print(((uint8_t*)&motorData)[i]);
      Serial.print(" ");
    }
    Serial.println();

    // Extract speed for the specific motor
    int inputSpeed = motorData.speed;

    // Verify if the speed is received correctly
    if (inputSpeed >= 0 && inputSpeed <= 100) {
      // Map the received speed to the desired range
      int mappedSpeed = mapSpeed(inputSpeed);

      // Preserve the direction while updating speed
      if (!isDirectionPositive) {
        mappedSpeed = -mappedSpeed;
      }

      // Set the speed for the stepper motor
      speed = mappedSpeed;
      stepper.setSpeed(speed);

      // Update the last packet timestamp
      lastPacketTime = millis();

      // Print the received data
      Serial.print("Motor ID: ");
      Serial.println(motorId);
      Serial.print("Input Speed (0-100): ");
      Serial.println(inputSpeed);
      Serial.print("Mapped Speed (300-1000): ");
      Serial.println(mappedSpeed);
    } else {
      Serial.println("Invalid speed received");
    }
  }

  // Check if timeout has occurred
  if (millis() - lastPacketTime > timeoutDuration) {
    // Set speed to default if no packet is received in the timeout period
    if (abs(speed) != defaultSpeed) {
      speed = isDirectionPositive ? defaultSpeed : -defaultSpeed;
      stepper.setSpeed(speed);
      Serial.println("No UDP packet received, setting speed to 500");
    }
  }

  // Run the stepper motor
  stepper.runSpeed();
}

void performAction1() {
  // Reverse the motor direction for END_SWITCH_1
  speed = -speed; // Reverse direction
  isDirectionPositive = !isDirectionPositive; // Update direction flag
  stepper.setSpeed(speed); // Update motor speed
  Serial.println("Reversing direction due to END_SWITCH_1");
  digitalWrite(BLINK_LED_PIN, HIGH);  // Turn on LED to indicate action
  delay(100);  // Brief delay for visual feedback
  digitalWrite(BLINK_LED_PIN, LOW);   // Turn off LED
}

void performAction2() {
  // Reverse the motor direction for END_SWITCH_2
  speed = -speed; // Reverse direction
  isDirectionPositive = !isDirectionPositive; // Update direction flag
  stepper.setSpeed(speed); // Update motor speed
  Serial.println("Reversing direction due to END_SWITCH_2");
  digitalWrite(BLINK_LED_PIN, HIGH);  // Turn on LED to indicate action
  delay(100);  // Brief delay for visual feedback
  digitalWrite(BLINK_LED_PIN, LOW);   // Turn off LED
}
