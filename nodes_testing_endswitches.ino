#include <AccelStepper.h>

// Define motor interface type
#define MotorInterfaceType 1

// Define pins
#define PUL_PIN D0               // Pulse pin for the stepper motor
#define DIR_PIN D1               // Direction pin for the stepper motor
#define ENA_PIN D2               // Enable pin for the stepper motor driver
#define END_SWITCH_1 D7          // End-switch 1
#define END_SWITCH_2 D6          // End-switch 2
#define BLINK_LED_PIN LED_BUILTIN // On-board LED

// Create a new instance of the AccelStepper class
AccelStepper stepper(MotorInterfaceType, PUL_PIN, DIR_PIN);

// Variables for LED blinking and motor control
bool buttonState1 = LOW;     // Current state of END_SWITCH_1
bool lastButtonState1 = LOW; // Previous state of END_SWITCH_1
bool actionTaken1 = false;   // Flag to check if action is taken for END_SWITCH_1

bool buttonState2 = LOW;     // Current state of END_SWITCH_2
bool lastButtonState2 = LOW; // Previous state of END_SWITCH_2
bool actionTaken2 = false;   // Flag to check if action is taken for END_SWITCH_2

unsigned long lastDebounceTime1 = 0; // Time the button state was last changed for END_SWITCH_1
unsigned long lastDebounceTime2 = 0; // Time the button state was last changed for END_SWITCH_2

const long debounceDelay = 50; // Debounce delay in milliseconds
const long blockTime = 5000;   // Block time in milliseconds

long speed = 500; // Motor speed

void setup() {
  pinMode(ENA_PIN, OUTPUT);
  pinMode(BLINK_LED_PIN, OUTPUT);
  pinMode(END_SWITCH_1, INPUT_PULLUP);
  pinMode(END_SWITCH_2, INPUT_PULLUP);

  digitalWrite(ENA_PIN, LOW); // Enable the driver

  // Set the maximum speed and acceleration
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);
  speed = 500; // Initial speed
}

void loop() {
  unsigned long currentMillis = millis();

  // Handle END_SWITCH_1
  int reading1 = digitalRead(END_SWITCH_1);
  if (reading1 != lastButtonState1) {
    lastDebounceTime1 = millis(); // reset debounce timer
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
    lastDebounceTime2 = millis(); // reset debounce timer
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

  // Set motor speed based on direction
  stepper.setSpeed(speed);

  // Run the stepper motor
  stepper.runSpeed();
}

void performAction1() {
  // Reverse the motor direction or any specific action for END_SWITCH_1
  speed = -speed;
  digitalWrite(BLINK_LED_PIN, HIGH);  // Turn on LED to indicate action
  delay(100);  // Brief delay for visual feedback
  digitalWrite(BLINK_LED_PIN, LOW);   // Turn off LED
}

void performAction2() {
  // Reverse the motor direction or any specific action for END_SWITCH_2
  speed = -speed;
  digitalWrite(BLINK_LED_PIN, HIGH);  // Turn on LED to indicate action
  delay(100);  // Brief delay for visual feedback
  digitalWrite(BLINK_LED_PIN, LOW);   // Turn off LED
}
