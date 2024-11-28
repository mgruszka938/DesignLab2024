// Constants for your stepper motor
const int stepsPerRevolution = 200; // Full steps per revolution (adjust for microstepping)

// Define pin connections
const int stepPin = 2; // A4988 STEP pin
const int dirPin = 5;  // A4988 DIR pin

// Variables for position tracking
long currentPosition = 0; // Track the current position in steps
long resetPosition = 0;   // Reference position

// Function prototypes
void processCommand(String command);
void moveMotor(int steps);

void setup() {
  // Initialize Serial communication
  Serial.begin(9600);
  Serial.println("A4988 Stepper Motor Controller Ready.");
  Serial.println("Commands: MOVE LEFT <steps>, MOVE RIGHT <steps>, RESET, POSITION");

  // Set pin modes
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  // Initial states
  digitalWrite(stepPin, LOW);
  digitalWrite(dirPin, LOW);
}

void loop() {
  // Check if there's any serial data available
  if (Serial.available() > 0) {
    // Read the incoming command
    String command = Serial.readStringUntil('\n');
    command.trim(); // Remove any leading/trailing whitespace

    // Process the command
    processCommand(command);
  }
}

// Function to process user commands
void processCommand(String command) {
  command.toUpperCase(); // Make it case-insensitive

  if (command.startsWith("MOVE LEFT")) {
    int steps = command.substring(10).toInt();
    if (steps > 0) {
      moveMotor(-steps);
    } else {
      Serial.println("Invalid step count!");
    }
  } else if (command.startsWith("MOVE RIGHT")) {
    int steps = command.substring(11).toInt();
    if (steps > 0) {
      moveMotor(steps);
    } else {
      Serial.println("Invalid step count!");
    }
  } else if (command.equalsIgnoreCase("RESET")) {
    // Calculate steps to return to zero position
    long stepsToZero = resetPosition - currentPosition;

    if (stepsToZero != 0) {
      // Move motor to zero position
      moveMotor(stepsToZero);
      Serial.println("Position reset.");
    } else {
      Serial.println("Motor already at zero position.");
    }

    // Update position tracking
    resetPosition = 0;
    currentPosition = 0;
  } else if (command == "POSITION") {
    Serial.print("Current Position: ");
    Serial.println(currentPosition - resetPosition);
  } else {
    Serial.println("Unknown command. Use MOVE LEFT, MOVE RIGHT, RESET, or POSITION.");
  }
}

// Function to move the motor
void moveMotor(int steps) {
  // Set direction
  if (steps > 0) {
    digitalWrite(dirPin, HIGH); // Move right
  } else {
    digitalWrite(dirPin, LOW); // Move left
    steps = -steps; // Convert to positive for the loop
  }

  // Move the motor step by step
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500); // Adjust this delay for speed (500 µs = ~1kHz step rate)
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }

  // Update position
  currentPosition += (steps * (digitalRead(dirPin) == HIGH ? 1 : -1));

  Serial.print("Motor moved ");
  Serial.print(steps);
  Serial.println(digitalRead(dirPin) == HIGH ? " steps right." : " steps left.");
}
