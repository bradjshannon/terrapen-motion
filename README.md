# TerraPen Motion Control Library

Arduino library for controlling differential drive wheeled robots with stepper motors and servo pen control.

## Current Status: Phase 1.5 Complete ✅

**Latest Version**: 1.1.0 (Phase 1.5)  
**Implementation Status**: Step-based robot control with coordinated movement

## Features

### Phase 1.5 - Complete Robot Control
- **TerraPenRobot**: Complete robot class with state machine and coordinated movement
- **Step-based Movement**: `moveForward()`, `moveBackward()`, `turnLeft()`, `turnRight()`
- **State Management**: IDLE, MOVING, ERROR, EMERGENCY_STOP states
- **Pen Control**: Integrated pen up/down with state tracking
- **Safety Features**: Emergency stop, error recovery, busy state protection

### Phase 1 - Hardware Drivers (Foundation)
- **StepperDriver**: Non-blocking control of 28BYJ-48 stepper motors with precise timing
- **ServoDriver**: Smooth servo movement with state tracking for pen control
- **RobotConfig**: Centralized hardware configuration management  
- **Position**: 2D coordinate system with utility functions
- **Arduino Library**: Proper library structure with examples and documentation

## Hardware Requirements

- Arduino Nano or compatible microcontroller
- Two 28BYJ-48 stepper motors with gear reduction
- ULN2803A or similar driver IC for stepper control
- Servo motor for pen up/down mechanism  
- 5V power supply (adequate current capacity)

## Hardware Setup

### Stepper Motors
```
Left Motor:     Right Motor:
IN1 -> Pin 2    IN1 -> Pin 6
IN2 -> Pin 3    IN2 -> Pin 7  
IN3 -> Pin 4    IN3 -> Pin 8
IN4 -> Pin 5    IN4 -> Pin 10
```

### Servo Motor
```
Signal -> Pin 9
Power  -> 5V
Ground -> GND
```

### Power Connections
- 5V and GND to stepper motors via ULN2803A
- 5V and GND to servo motor
- Common ground for all components

## Installation

1. Download or clone this repository
2. Copy the entire folder to your Arduino libraries directory:
   - Windows: `Documents\Arduino\libraries\terrapen-motion`
   - Mac: `~/Documents/Arduino/libraries/terrapen-motion`
   - Linux: `~/Arduino/libraries/terrapen-motion`
3. Restart Arduino IDE
4. Library will appear under `Sketch > Include Library > TerraPen Motion Control`

## Basic Usage

### Recommended: Phase 1.5 Robot Control

```cpp
#include <TerraPenMotionControl.h>

TerraPenRobot robot;
RobotConfig config;

void setup() {
  Serial.begin(115200);
  
  // Initialize robot with default configuration
  robot.begin(config);
  
  Serial.println("Robot ready!");
}

void loop() {
  // Always call update() to coordinate hardware
  robot.update();
  
  // Execute movements when robot is not busy
  if (!robot.isBusy()) {
    robot.moveForward(50);    // Move 50 steps forward
    delay(1000);              // Pause between commands
    
    robot.penDown();          // Lower pen
    robot.turnLeft(25);       // Turn left 25 steps  
    robot.penUp();            // Raise pen
    
    robot.moveBackward(25);   // Move back 25 steps
  }
}
```

### Advanced: Direct Hardware Control (Phase 1)

```cpp
#include <TerraPenMotionControl.h>

// Create hardware drivers
StepperDriver left_motor;
StepperDriver right_motor;
ServoDriver pen_servo;

// Use default configuration
RobotConfig config;

void setup() {
  Serial.begin(115200);
  
  // Initialize hardware
  left_motor.begin(config.left_motor_pins[0], config.left_motor_pins[1],
                   config.left_motor_pins[2], config.left_motor_pins[3]);
  right_motor.begin(config.right_motor_pins[0], config.right_motor_pins[1],
                    config.right_motor_pins[2], config.right_motor_pins[3]);
  pen_servo.begin(config.servo_pin, config.pen_up_angle);
  
  // Set motor speeds (steps per second)
  left_motor.setSpeed(100);
  right_motor.setSpeed(100);
}

void loop() {
  // Move both motors forward (robot goes straight)
  left_motor.stepForward();
  right_motor.stepForward();
  
  // Update servo for smooth movement
  pen_servo.update();
}
```

## Examples

The library includes several example sketches:

### Phase 1.5 Examples (Recommended)
- **Phase1_5Integration**: Comprehensive test of robot control, state machine, and movement coordination
- **SimpleRobotTest**: Basic robot functionality with step-based movements

### Phase 1 Examples (Hardware Drivers)
- **StepperTest**: Basic stepper motor control and timing
- **ServoTest**: Servo movement and smooth transitions  
- **HardwareTest**: Combined operation of all hardware components

Open examples via `File > Examples > TerraPen Motion Control` in Arduino IDE.

## API Reference

### TerraPenRobot Class (Phase 1.5 - Recommended)

```cpp
void begin(const RobotConfig& config);           // Initialize robot with configuration

// Step-based movement commands (non-blocking)
bool moveForward(int steps);                     // Move both motors forward
bool moveBackward(int steps);                    // Move both motors backward  
bool turnLeft(int steps);                        // Differential turn left
bool turnRight(int steps);                       // Differential turn right

// Pen control
void penUp();                                    // Raise pen
void penDown();                                  // Lower pen
bool isPenDown() const;                          // Check pen state

// State management
RobotState getState() const;                     // Get current state (IDLE, MOVING, ERROR, EMERGENCY_STOP)
bool isBusy() const;                             // Check if robot is busy
void emergencyStop();                            // Immediate stop all movement
void clearError();                               // Clear error state

// Step tracking (for future position estimation)
long getLeftStepsTotal() const;                  // Get accumulated left steps
long getRightStepsTotal() const;                 // Get accumulated right steps
void resetStepCounts();                          // Reset step counters

void update();                                   // Call every loop - coordinates hardware
```

### StepperDriver Class (Phase 1)

```cpp
void begin(int in1, int in2, int in3, int in4);  // Initialize with pins
void setSpeed(float steps_per_sec);              // Set stepping speed
bool stepForward();                              // Non-blocking forward step
bool stepBackward();                             // Non-blocking backward step  
void stepNow(int direction);                     // Immediate step (blocking)
bool isReady();                                  // Check if ready for next step
void hold();                                     // Energize coils (hold position)
void release();                                  // Turn off coils (save power)
```

### ServoDriver Class

```cpp
void begin(int servo_pin, int initial_angle = 90); // Initialize servo
void setAngle(int degrees);                         // Immediate position
void sweepTo(int degrees, unsigned long duration);  // Smooth movement
int getCurrentAngle();                              // Get current position
bool isMoving();                                    // Check if moving
void update();                                      // Process smooth movement (call in loop)
```

### RobotConfig Struct

```cpp
// Physical parameters
float wheel_diameter_mm;      // Wheel diameter (default: 25mm)
float wheelbase_mm;           // Distance between wheels (default: 30mm) 
int steps_per_revolution;     // Motor steps per revolution (default: 2048)

// Pin assignments
int left_motor_pins[4];       // Left motor pins [IN1, IN2, IN3, IN4]
int right_motor_pins[4];      // Right motor pins [IN1, IN2, IN3, IN4]
int servo_pin;                // Servo signal pin (default: 9)

// Servo configuration  
int pen_up_angle;             // Pen up position (default: 90°)
int pen_down_angle;           // Pen down position (default: 0°)

// Utility methods
float getStepsPerMM();        // Calculate steps per millimeter
bool isInWorkspace(x, y);     // Check if coordinates are valid
void printConfig();           // Debug output to Serial
```

### Position Struct

```cpp
float x, y;                   // Coordinates in millimeters
float angle;                  // Orientation in radians

// Utility methods
float distanceTo(other);      // Distance to another position
float angleTo(other);         // Angle to another position
Position offsetBy(dx, dy);    // Create offset position
String toString();            // Convert to string for debugging
```

## Configuration

Customize hardware setup by modifying the RobotConfig:

```cpp
RobotConfig config;
config.wheel_diameter_mm = 30.0;        // Larger wheels
config.wheelbase_mm = 40.0;             // Wider robot
config.pen_up_angle = 120;              // Different servo angles
config.pen_down_angle = 30;
config.max_speed_mms = 25.0;            // Speed limits

// Custom pin assignments
config.left_motor_pins[0] = 10;         // Change pin assignments
config.servo_pin = 11;
```

## Differential Drive Kinematics

The library implements standard differential drive kinematics:

- **Forward movement**: Both motors step in same direction
- **Backward movement**: Both motors step in opposite direction  
- **Turn left**: Left motor backward, right motor forward
- **Turn right**: Left motor forward, right motor backward
- **Rotation in place**: Motors at equal speed in opposite directions

## Troubleshooting

### Motors don't move
- Check wiring connections to ULN2803A
- Verify 5V power supply and current capacity
- Test with individual motor using StepperTest example

### Irregular movement
- Check power supply stability (add capacitors if needed)
- Reduce motor speed with `setSpeed()`
- Verify stepper sequence in StepperDriver.cpp

### Servo doesn't respond  
- Check servo power connections (5V, GND)
- Verify signal wire connection to correct pin
- Test with ServoTest example

### System resets unexpectedly
- Power supply insufficient current capacity
- Add larger capacitors to smooth power delivery
- Check for short circuits in wiring

## Contributing

This library is part of the TerraPen project. Contributions welcome:

1. Fork the repository
2. Create feature branch
3. Test changes with provided examples  
4. Submit pull request with clear description

## License

[Add your license here]

## Hardware Specifications

### 28BYJ-48 Stepper Motor
- **Type**: Unipolar stepper with gear reduction
- **Steps per revolution**: 2048 (with gear reduction)  
- **Voltage**: 5V DC
- **Current**: ~300mA per phase
- **Torque**: ~300 g⋅cm (at low speeds)

### ULN2803A Driver IC
- **Type**: 8-channel Darlington transistor array
- **Input voltage**: 5V (TTL compatible)
- **Output current**: 500mA per channel
- **Protection**: Built-in flyback diodes

This library provides a solid foundation for building drawing robots, plotters, and similar applications requiring precise 2D movement control.