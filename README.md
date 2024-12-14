# Stepper Motor Laser Aim
## Necessary equipment
- Arduino Uno Rev3
- stepper motor NEMA 17 (bipolar)
- driver module A4988
- 12V power supply
## Schematic
![SVG Image](schematic.drawio.svg)
## Usage
Commands supported:
```
Commands:
MVL <steps> - move left
MVR <steps> - move right
RST - reset the position
POS - view current position
ADDPOS <name> - add a new position
GOTO <name> - go to <name> position
DLPOS <name> - delete <name> position
LSTPOS - list saved positions
```
## Timeline
- We've spent first couple of weeks on acquring knowledge about stepper motors.
- Then we focused on assembling our circuit, which was a bit challenging, because of lack of the power supply. Fortunately we found one (from an old router).
- Another task that we focused most of our time on was writing a functioning program that will properly move our stepper motor.
- We added appropriate functionality to enable UART communication.
- We added multiple move functions that we thought would be suitable for the darkbox environment.
- We extended the functionality by integrating EEPROM memory to save and retrieve motor positions, ensuring persistence across power cycles.
- We also adjusted the movement range to a safe limit of -100 to 100 steps to prevent mechanical issues.
- We plan on adding further components and functions if needed.
## Authors
- Mikołaj Gruszka
- Szymon Rumin

