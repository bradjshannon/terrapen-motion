# TerraPen Motion Control Library

Arduino library for controlling differential drive wheeled robots with stepper motors and servo pen control, featuring coordinate-based movement, comprehensive testing framework, performance monitoring, and quality assurance systems.

## Current Status: Phase 2 Complete ✅

**Latest Version**: 2.0.0  
**Implementation Status**: Complete coordinate-based robot control with differential drive kinematics, position tracking, workspace boundaries, and simple testing framework

**Phase 2 Implementation** (September 19, 2025):
- ✅ **Coordinate Movement**: `moveTo(x,y)`, `drawTo(x,y)` with automatic pen control
- ✅ **Differential Drive Kinematics**: Forward/inverse kinematics calculations for precise movement
- ✅ **Position Tracking**: Real-time position estimation based on step counts
- ✅ **Rotation Control**: `turnTo(angle)`, `turnBy(delta)` for precise angular positioning
- ✅ **Workspace Boundaries**: Automatic validation of movement commands within safe area
- ✅ **Integration Testing**: Comprehensive validation with existing quality framework

**VSCode Testing Integration** (September 26, 2025):
- ✅ **Native Test Explorer**: 23 mathematical validation tests in VSCode Test Explorer
- ✅ **Zero-Hardware Testing**: Core algorithms validated without physical hardware
- ✅ **Python Virtual Environment**: Isolated development environment with all dependencies
- ✅ **Test Discovery System**: Automated test enumeration and organization
- ✅ **Professional Workflow**: Seamless integration with VSCode development environment

## Features

### Phase 2 - Coordinate-Based Movement (NEW)
- **Coordinate System**: Full 2D coordinate support with `moveTo(x,y)` and `drawTo(x,y)` methods
- **Differential Drive Kinematics**: Precise forward/inverse kinematics for accurate movement
- **Position Tracking**: Real-time position estimation based on wheel encoder step counts
- **Rotation Control**: `turnTo()` and `turnBy()` methods for precise angular positioning
- **Workspace Boundaries**: Automatic validation prevents movement outside safe operating area
- **Automatic Pen Control**: Movement commands automatically handle pen up/down states

### Phase 1.6 - Quality & Testing Framework
- **Comprehensive Testing**: 280+ test cases covering all system components
- **Multiple Test Modes**: Quick POST (2s), Full POST (15s), Unit Tests, Hardware Tests, Integration Tests
- **Testing Framework**: Custom Arduino unit testing with assertion macros and test tagging
- **Power-On Self Test (POST)**: Runtime validation of critical systems (Quick & Full modes)
- **Performance Monitoring**: Real-time CPU, memory, timing, and step rate tracking
- **NVRAM Data Logging**: Circular buffer storage with ESP32 upload capability
- **Error Management**: 60+ specific error codes with context tracking and recovery
- **Centralized Configuration**: Single configuration file for all project settings
- **Test Automation**: Git hooks, CI/CD integration, and automated quality gates
- **Interactive Testing**: Serial command interface for manual test execution

### Phase 1.5 - Complete Robot Control
- **TerraPenRobot**: Complete robot class with state machine and coordinated movement
- **Step-based Movement**: `moveForward()`, `moveBackward()`, `turnLeft()`, `turnRight()`
- **State Management**: IDLE, MOVING, ERROR, EMERGENCY_STOP states
- **Pen Control**: Integrated pen up/down with state tracking
- **Safety Features**: Emergency stop, error recovery, busy state protection

### Phase 1 - Hardware Drivers (Foundation)
- **StepperDriver**: Non-blocking control of 28BYJ-48 stepper motors with precise timing
- **ServoDriver**: Smooth servo movement with state tracking for pen control
- **TerraPenConfig**: Hardware configuration management with comprehensive settings
- **Position**: 2D coordinate system with utility functions
- **Arduino Library**: Proper library structure with examples and documentation

## Hardware Requirements

- Arduino Nano or compatible microcontroller
- Two 28BYJ-48 stepper motors with gear reduction
- ULN2803A or similar driver IC for stepper control
- Servo motor for pen up/down mechanism  
- 5V power supply (adequate current capacity)
- (Optional) ESP32 for data upload and extended connectivity

## Hardware Setup

### Stepper Motors
```
Left Motor:     Right Motor:
IN1 -> Pin 2    IN1 -> Pin 6
IN2 -> Pin 3    IN2 -> Pin 7  
IN3 -> Pin 4    IN3 -> Pin 8
IN4 -> Pin 5    IN4 -> Pin 9
```

### Servo Motor
```
Signal -> Pin 10
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

### Recommended: Phase 2 Coordinate-Based Movement

```cpp
#include <TerraPenMotionControl.h>

TerraPenRobot robot;

void setup() {
  Serial.begin(115200);
  
  // Initialize robot with global configuration  
  robot.begin();
  
  // Set starting position (coordinate system origin)
  robot.resetPosition(0, 0, 0);  // x=0, y=0, angle=0
  
  Serial.println("Robot ready for coordinate movement!");
}

void loop() {
  // Always call update() to coordinate hardware
  robot.update();
  
  // Execute movements when robot is not busy
  if (!robot.isBusy()) {
    // Draw a 20mm x 20mm square
    robot.moveTo(0, 0);       // Move to start (pen automatically up)
    robot.drawTo(20, 0);      // Draw right side (pen automatically down)
    robot.drawTo(20, 20);     // Draw top side
    robot.drawTo(0, 20);      // Draw left side  
    robot.drawTo(0, 0);       // Complete square
    
    // Move to new location and draw triangle
    robot.moveTo(30, 30);     // Move to new position (pen up)
    robot.drawTo(45, 30);     // Base of triangle (pen down)
    robot.drawTo(37.5, 43);   // Peak of triangle
    robot.drawTo(30, 30);     // Close triangle
    
    // Print current position
    Position pos = robot.getCurrentPosition();
    Serial.print("Current position: ");
    pos.print();  // Prints: "Position: (30.00, 30.00) @ 0.0°"
    
    delay(3000);  // Pause before repeating
  }
}
```

### Alternative: Phase 1.5 Step-Based Movement

```cpp
#include <TerraPenMotionControl.h>

TerraPenRobot robot;

void setup() {
  Serial.begin(115200);
  robot.begin();
  Serial.println("Robot ready!");
}

void loop() {
  robot.update();
  
  if (!robot.isBusy()) {
    robot.moveForward(50);    // Move 50 steps forward
    delay(1000);              
    
    robot.penDown();          // Lower pen
    robot.turnLeft(25);       // Turn left 25 steps  
    robot.penUp();            // Raise pen
    
    robot.moveBackward(25);   // Move back 25 steps
  }
}
```

## Quality Assurance & Testing

### Testing Framework
The library includes a comprehensive testing framework for Arduino:

```cpp
#include <TerraPenMotionControl.h>

void setup() {
  Serial.begin(115200);
  
  // Run Power-On Self Test (POST)
  PostResults results = runQuickPost();
  if (!results.passed) {
    Serial.println("POST Failed!");
    Serial.println(results.failure_summary);
    return;
  }
  
  // Initialize performance monitoring
  performance_monitor.begin();
  
  // Initialize robot with validated configuration
  TerraPenRobot robot;
  robot.begin();
}

void loop() {
  // Normal robot operation with monitoring
  robot.moveForward(100);
  
  // Check performance metrics periodically
  if (millis() % 10000 == 0) {  // Every 10 seconds
    PerformanceMetrics metrics = performance_monitor.getMetrics();
    Serial.print("CPU: "); Serial.print(metrics.cpu_usage_percent); Serial.println("%");
    Serial.print("Free RAM: "); Serial.print(metrics.free_memory_bytes); Serial.println(" bytes");
  }
}
```

### Interactive Test Runner
Use the TestRunner example for comprehensive system validation:

```
File → Examples → TerraPenMotionControl → TestRunner
```

Interactive menu provides:
- **Unit Tests**: Complete test suite validation
- **POST Tests**: Power-on self test execution
- **Performance Monitoring**: Real-time system metrics
- **Storage Status**: NVRAM usage and upload status
- **Configuration Display**: Current system settings

### Error Handling
Structured error management with detailed context:

```cpp
void loop() {
  // Check for system errors
  if (HAS_ERROR()) {
    ErrorCode error = g_error_manager.getCurrentErrorCode();
    String context = g_error_manager.getCurrentContext();
    
    Serial.print("Error "); Serial.print(error); 
    Serial.print(": "); Serial.println(context);
    
    // Attempt automatic recovery
    if (g_config.error_handling.enable_auto_recovery) {
      CLEAR_ERROR();
      robot.emergencyStop();
      delay(1000);
      robot.reset();
    }
  }
}
```

### Performance Data Upload
Automatic ESP32 data upload (when configured):

```cpp
void setup() {
  // Enable ESP32 communication (optional)
  g_config.communication.enable_esp32_upload = true;
  g_nvram_manager.begin(&esp32_uploader);
}

// Performance data is automatically:
// 1. Stored to NVRAM every 10 seconds
// 2. Uploaded to ESP32 when connected
// 3. Cleaned up when upload confirmed
```

### Advanced: Direct Hardware Control (Phase 1)

```cpp
#include <TerraPenMotionControl.h>

// Create hardware drivers
StepperDriver left_motor;
StepperDriver right_motor;
ServoDriver pen_servo;

// Use default configuration
TerraPenConfig config;

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
void begin();                                    // Initialize robot with global configuration

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

### TerraPenConfig Struct

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

Customize hardware setup by modifying the TerraPenConfig:

```cpp
// Access global configuration instance
g_config.hardware.wheel_diameter_mm = 30.0;        // Larger wheels
g_config.hardware.wheelbase_mm = 40.0;             // Wider robot
g_config.hardware.pen_up_angle = 120;              // Different servo angles
g_config.hardware.pen_down_angle = 30;
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

## 📚 Documentation

Complete documentation is available in the [`docs/`](docs/) folder:

- **[API Reference](docs/API_REFERENCE.md)** - Complete API documentation for all classes and methods
- **[System Architecture](docs/ARCHITECTURE.md)** - System design, patterns, and component relationships
- **[Testing Guide](docs/TESTING.md)** - How to run tests and validate your robot code
- **[VSCode Testing Integration](docs/VSCODE_TESTING_INTEGRATION.md)** - Complete setup guide for VSCode testing
- **[Development Setup](docs/DEVELOPMENT_SETUP.md)** - Development environment setup and workflow
- **[Performance Strategy](docs/ADAPTIVE_PERFORMANCE_STRATEGY.md)** - Real-time monitoring and optimization
- **[Phase 2 Status](docs/PHASE_2_COMPLETE.md)** - Current implementation status and coordinate system features
- **[Automation Setup](docs/AUTOMATION_SETUP.md)** - CI/CD integration and development workflow
- **[EEPROM Strategy](docs/EEPROM_STRATEGY.md)** - Data storage and persistence
- **[POST Optimization](docs/POST_OPTIMIZATION.md)** - Power-on self-test configuration

See [`docs/README.md`](docs/README.md) for complete documentation index.