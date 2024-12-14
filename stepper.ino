/*
 * Stepper Motor Controller
 *
 * Authors: Mikołaj Gruszka, Szymon Rumin
 * Created for: Design Laboratory
 *
 * Description:
 * This program is designed to control a stepper motor using the A4988 driver.
 * It supports commands sent over UART for moving the motor, resetting its position,
 * and querying its current position. The motor's movement is tracked internally.
 *
 * Commands:
 * MVL <steps> - move left
 * MVR <steps> - move right
 * RST - reset the position
 * POS - view current position
 * ADDPOS <name> - add a new position
 * GOTO <name> - go to <name> position
 * DLPOS <name> - delete <name> position
 * LSTPOS - list saved positions
 */

// EEPROM library
#include <EEPROM.h>

// EEPROM constants
const int NAME_SIZE = 11;
const int RECORD_SIZE = 4 + NAME_SIZE;

// Constants
const int stepsPerRevolution = 200; // could be adjusted for microstepping
const int maxPositions = 10;

// Pin connections
const int stepPin = 2; // A4988 STEP
const int dirPin = 5;  // A4988 DIR

// Keeps track of the position
long currentPosition = 0;
long resetPosition = 0;

// Arrays for named positions
String positionNames[maxPositions];
long positionValues[maxPositions];
int positionCount = 0;

// Function prototypes
void processCommand(String command);
void moveMotor(int steps);
void addNamedPosition(String name);
void moveToNamedPosition(String name);
void deleteNamedPosition(String name);
int findNamedPosition(String name);

void setup()
{
  // Initialize Serial COM
  Serial.begin(9600);
  Serial.println("A4988 Stepper Motor Controller Ready.");

  // Set pin modes
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  // Initial states
  digitalWrite(stepPin, LOW);
  digitalWrite(dirPin, LOW);

  // Load saved positions and print avaliable commands
  printCommands();
  loadFromEEPROM();

  // Read stepper position from EEPROM
  Serial.print("Current stepper position: ");
  Serial.println(currentPosition);


}

void loop()
{
  // Check if there's any serial data available
  if (Serial.available() > 0)
  {
    // Read the incoming command
    String command = Serial.readStringUntil('\n');
    command.trim();

    // Process the command
    processCommand(command);
  }
}

// Function to process user commands
void processCommand(String command)
{
  command.toUpperCase(); // Upper case (case insensitive commands)

  if (command.startsWith("MVL"))
  {
    int steps = command.substring(4).toInt();
    if (steps > 0)
    {
      if (currentPosition - steps >= -100)
      {
        moveMotor(-steps);
        saveToEEPROM();
      }
      else
      {
        Serial.println("Movement exceeds allowed range (-100 to 100).");
      }
    }
    else
    {
      Serial.println("Invalid step count!");
    }
  }
  else if (command.startsWith("MVR"))
  {
    int steps = command.substring(4).toInt();
    if (steps > 0)
    {
      if (currentPosition + steps <= 100)
      {
        moveMotor(steps);
        saveToEEPROM();
      }
      else
      {
        Serial.println("Movement exceeds allowed range (-100 to 100).");
      }
    }
    else
    {
      Serial.println("Invalid step count!");
    }
  }
  else if (command.equalsIgnoreCase("RST"))
  {
    long stepsToZero = resetPosition - currentPosition;

    if (stepsToZero != 0)
    {
      moveMotor(stepsToZero);
      saveToEEPROM();
      Serial.println("Position reset.");
    }
    else
    {
      Serial.println("Motor already at zero position.");
    }

    resetPosition = 0;
    currentPosition = 0;
  }
  else if (command == "POS")
  {
    Serial.print("Current Position: ");
    Serial.println(currentPosition - resetPosition);
  }
  else if (command.startsWith("ADDPOS"))
  {
    String name = command.substring(7);
    addNamedPosition(name);
  }
  else if (command.startsWith("GOTO"))
  {
    String name = command.substring(5);
    moveToNamedPosition(name);
  }
  else if (command.startsWith("DLPOS"))
  {
    String name = command.substring(6);
    deleteNamedPosition(name);
  }
  else if (command == "LSTPOS")
  {
    listNamedPositions();
  }
  else if (command == "?")
  {
    printCommands();
  }
  else
  {
    Serial.println("Unknown command");
  }
}

void printCommands()
{
  Serial.println("Commands:");
  Serial.println("MVL <steps> - move left");
  Serial.println("MVR <steps> - move right");
  Serial.println("RST - reset the position");
  Serial.println("POS - view current position");
  Serial.println("ADDPOS <name> - add a new position");
  Serial.println("GOTO <name> - go to <name> position");
  Serial.println("DLPOS <name> - delete <name> position");
  Serial.println("LSTPOS - list saved positions");
}

// Function to move the motor
void moveMotor(int steps)
{
  if (steps == 0)
    return;

  if (steps > 0)
  {
    digitalWrite(dirPin, HIGH); // Move right
  }
  else
  {
    digitalWrite(dirPin, LOW); // Move left
    steps = -steps;
  }

  for (int i = 0; i < steps; i++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(10000);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(10000);
  }

  currentPosition += (steps * (digitalRead(dirPin) == HIGH ? 1 : -1));

  Serial.print("Motor moved ");
  Serial.print(steps);
  Serial.println(digitalRead(dirPin) == HIGH ? " steps right." : " steps left.");

}

// Function to add a named position
void addNamedPosition(String name)
{
  if (positionCount >= maxPositions)
  {
    Serial.println("Cannot add more positions. Maximum reached.");
    return;
  }

  if (findNamedPosition(name) != -1)
  {
    Serial.println("Position name already exists! Use a unique name.");
    return;
  }

  if (name.length() > NAME_SIZE - 1) {
    Serial.println("Position name is too long.");
    return;
  }

  positionNames[positionCount] = name;
  positionValues[positionCount] = currentPosition - resetPosition;
  positionCount++;
  saveToEEPROM();
  Serial.print("Position '");
  Serial.print(name);
  Serial.println("' saved.");
}

// Function to move to a named position
void moveToNamedPosition(String name)
{
  int index = findNamedPosition(name);
  if (index == -1)
  {
    Serial.print("Position '");
    Serial.print(name);
    Serial.println("' not found.");
    return;
  }

  long targetPosition = positionValues[index];
  long currentRelative = currentPosition - resetPosition;

  long directSteps = targetPosition - currentRelative;

  long clockwiseSteps = (directSteps >= 0) ? directSteps : (200 + directSteps);
  long counterClockwiseSteps = (directSteps <= 0) ? directSteps : (-200 + directSteps);

  long shortestSteps = abs(clockwiseSteps) < abs(counterClockwiseSteps) ? clockwiseSteps : counterClockwiseSteps;

  moveMotor(shortestSteps);
  saveToEEPROM();
  Serial.print("Moved to position '");
  Serial.print(name);
  Serial.println("' using shortest path.");
}

// Function to delete a named position
void deleteNamedPosition(String name) {
  int index = findNamedPosition(name);
  if (index == -1) {
    Serial.print("Position '");
    Serial.print(name);
    Serial.println("' not found.");
    return;
  }

  for (int i = index; i < positionCount - 1; i++) {
    positionNames[i] = positionNames[i + 1];
    positionValues[i] = positionValues[i + 1];
  }
  positionCount--;

  saveToEEPROM();

  Serial.print("Position '");
  Serial.print(name);
  Serial.println("' deleted.");
}

// Function to find a named position
int findNamedPosition(String name)
{
  for (int i = 0; i < positionCount; i++)
  {
    if (positionNames[i] == name)
    {
      return i;
    }
  }
  return -1;
}

// Function to display all saved positions
void listNamedPositions()
{
  if (positionCount == 0)
  {
    Serial.println("No saved positions.");
    return;
  }

  Serial.println("Saved positions:");
  for (int i = 0; i < positionCount; i++)
  {
    Serial.print("Name: ");
    Serial.print(positionNames[i]);
    Serial.print(", Position: ");
    Serial.println(positionValues[i]);
  }
}

// Function to save program data to EEPROM memory
void saveToEEPROM() {
  static long lastSavedPosition = -9999;

  if (currentPosition != lastSavedPosition) {
    EEPROM.put(0, currentPosition);

  for (int i = 0; i < maxPositions; i++) {
    int address = 4 + i * RECORD_SIZE;

    if (i < positionCount) {
      EEPROM.put(address, positionValues[i]);

      for (int j = 0; j < NAME_SIZE; j++) {
        char c = (j < positionNames[i].length()) ? positionNames[i][j] : '\0';
        EEPROM.write(address + 4 + j, c);
      }
    } else {
      EEPROM.put(address, 0L);
      for (int j = 0; j < NAME_SIZE; j++) {
        EEPROM.write(address + 4 + j, '\0');
      }
    }
  }

    lastSavedPosition = currentPosition;

  }
}

// Function to load program data from EEPROM memory
void loadFromEEPROM()
{
  EEPROM.get(0, currentPosition);

  // Check if the loaded position is in a valid range, otherwise reset to 0
  if (currentPosition < -100 || currentPosition > 100) {
    Serial.println("Invalid position in EEPROM, resetting to 0.");
    currentPosition = 0;  // Reset to 0 if the position is invalid
  }

  positionCount = 0;

  // Load saved named positions from EEPROM
  for (int i = 0; i < maxPositions; i++) {
    int address = 4 + i * RECORD_SIZE;

    long value;
    EEPROM.get(address, value);

    char name[NAME_SIZE];
    for (int j = 0; j < NAME_SIZE; j++) {
      name[j] = EEPROM.read(address + 4 + j);
    }
    name[NAME_SIZE - 1] = '\0';

    if (name[0] == '\0') {
      break;
    }

    positionNames[positionCount] = String(name);
    positionValues[positionCount] = value;
    positionCount++;

    if (positionCount >= maxPositions) {
      break;
    }
  }
}
