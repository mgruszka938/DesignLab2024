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
 * Commands supported:
 * - MOVE LEFT <steps>
 * - MOVE RIGHT <steps>
 * - RESET
 * - POSITION
 * - ADD POS <name>  
 * - GOTO <name>
 * - DEL POS <name>
 */

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
  Serial.println("Commands: MOVE LEFT <steps>, MOVE RIGHT <steps>, RESET, POSITION, ADD POS <name>, GOTO <name>, DEL POS <name>");

  // Set pin modes
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  // Initial states
  digitalWrite(stepPin, LOW);
  digitalWrite(dirPin, LOW);
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

  if (command.startsWith("MOVE LEFT"))
  {
    int steps = command.substring(10).toInt();
    if (steps > 0)
    {
      moveMotor(-steps);
    }
    else
    {
      Serial.println("Invalid step count!");
    }
  }
  else if (command.startsWith("MOVE RIGHT"))
  {
    int steps = command.substring(11).toInt();
    if (steps > 0)
    {
      moveMotor(steps);
    }
    else
    {
      Serial.println("Invalid step count!");
    }
  }
  else if (command.equalsIgnoreCase("RESET"))
  {
    long stepsToZero = resetPosition - currentPosition;

    if (stepsToZero != 0)
    {
      moveMotor(stepsToZero);
      Serial.println("Position reset.");
    }
    else
    {
      Serial.println("Motor already at zero position.");
    }

    resetPosition = 0;
    currentPosition = 0;
  }
  else if (command == "POSITION")
  {
    Serial.print("Current Position: ");
    Serial.println(currentPosition - resetPosition);
  }
  else if (command.startsWith("ADD POS"))
  {
    String name = command.substring(8);
    addNamedPosition(name);
  }
  else if (command.startsWith("GOTO"))
  {
    String name = command.substring(5);
    moveToNamedPosition(name);
  }
  else if (command.startsWith("DEL POS"))
  {
    String name = command.substring(8);
    deleteNamedPosition(name);
  }
  else
  {
    Serial.println("Unknown command. Use MOVE LEFT, MOVE RIGHT, RESET, POSITION, ADD POS <name>, GOTO <name>, DEL POS <name>.");
  }
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
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
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

  positionNames[positionCount] = name;
  positionValues[positionCount] = currentPosition - resetPosition;
  positionCount++;

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
  long stepsToMove = targetPosition - (currentPosition - resetPosition);
  moveMotor(stepsToMove);

  Serial.print("Moved to position '");
  Serial.print(name);
  Serial.println("'.");
}

// Function to delete a named position
void deleteNamedPosition(String name)
{
  int index = findNamedPosition(name);
  if (index == -1)
  {
    Serial.print("Position '");
    Serial.print(name);
    Serial.println("' not found.");
    return;
  }

  for (int i = index; i < positionCount - 1; i++)
  {
    positionNames[i] = positionNames[i + 1];
    positionValues[i] = positionValues[i + 1];
  }
  positionCount--;

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
