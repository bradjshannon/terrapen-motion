# TerraPen Motion Control Architecture (Revised)

**Version**: 2.0  
**Date**: September 18, 2025  
**Project**: Differential drive drawing robot with Arduino Nano + ESP32 controller

## System Overview

The TerraPen system uses a **simplified 3-layer architecture** focused on real-world effectiveness:

- **L1 (Hardware)**: Direct motor/servo control with timing
- **L2 (Robot)**: Kinematics, movement primitives, and robot state
- **L3 (Communication)**: ESP32↔Nano protocol and command processing

**Design Philosophy**: Simple, robust, and aligned with standard embedded/robotics practices.

## Communication Architecture

```
Studio (Python) ←─WiFi─→ ESP32 Controller ←─UART─→ Arduino Nano ←─pins─→ Hardware
     ↑                        ↑                      ↑
 Path Planning          Command Queuing        Motion Execution
 Visualization          Multi-robot Coord      Hardware Control
 Simulation             Trajectory Planning    Real-time Control
```

## Communication Protocol

### Message Format (JSON over Serial)
```json
// Command from ESP32 to Nano
{
  "id": 123,
  "cmd": "MOVE_TO",
  "x": 50.5,
  "y": 30.2,
  "speed": 15.0,
  "pen": false
}

// Status from Nano to ESP32
{
  "ack": 123,
  "x": 45.1,
  "y": 28.9,
  "angle": 1.57,
  "pen": false,
  "busy": true,
  "error": null
}
```

### Command Types
- `MOVE_TO`: Navigate to coordinates (pen up)
- `DRAW_TO`: Draw line to coordinates (pen down)
- `PEN_UP` / `PEN_DOWN`: Control pen position
- `GET_STATUS`: Request current state
- `STOP`: Emergency stop
- `RESET`: Return to origin

---

## Layer 1: Hardware Drivers

### StepperDriver
**Purpose**: Direct stepper motor control with precise timing  
**File**: `src/hardware/StepperDriver.h/cpp`

```cpp
class StepperDriver {
private:
    int pins[4];
    int current_phase;           // 0-7 for half-step sequence
    unsigned long last_step_us;
    unsigned long step_interval_us;
    
    static const int PHASE_SEQUENCE[8][4];  // 28BYJ-48 half-step
    
public:
    // Configuration
    void begin(int in1, int in2, int in3, int in4);
    void setSpeed(float steps_per_sec);
    
    // Stepping control
    bool stepForward();          // Non-blocking, returns true if stepped
    bool stepBackward();         // Non-blocking, returns true if stepped
    void stepNow(int direction); // Immediate step (blocking)
    
    // Motor control
    void hold();                 // Energize coils to hold position
    void release();              // Turn off all coils
    
    // State
    bool isReady();              // Ready for next step
    
private:
    void applyPhase();
};
```

**Implementation Status**: 🔴 Not started  
**Dependencies**: Arduino core  
**Testing**: Phase sequence logic, timing accuracy

---

### ServoDriver
**Purpose**: Servo motor control for pen mechanism  
**File**: `src/hardware/ServoDriver.h/cpp`

```cpp
class ServoDriver {
private:
    int pin;
    int current_angle;
    int target_angle;
    unsigned long move_start_time;
    unsigned long move_duration;
    bool is_moving;
    
public:
    // Configuration
    void begin(int servo_pin);
    
    // Position control
    void setAngle(int degrees);                    // Immediate
    void sweepTo(int degrees, unsigned long ms);   // Smooth movement
    
    // State
    bool isMoving();
    int getCurrentAngle();
    
    // Update (call in main loop)
    void update();
    
private:
    void writeAngle(int angle);
};
```

**Implementation Status**: 🔴 Not started  
**Dependencies**: Arduino Servo library  
**Testing**: State tracking, smooth movement logic

---

## Layer 2: Robot Control

### TerraPenRobot
**Purpose**: Complete robot control with kinematics and state management  
**File**: `src/robot/TerraPenRobot.h/cpp`

```cpp
enum RobotState {
    IDLE,           // Ready for commands
    MOVING,         // Executing movement
    DRAWING,        // Drawing (pen down + moving)
    ERROR,          // Error state
    EMERGENCY_STOP  // Emergency stop engaged
};

class TerraPenRobot {
private:
    // Hardware drivers
    StepperDriver left_motor;
    StepperDriver right_motor;
    ServoDriver pen_servo;
    
    // Robot configuration
    float wheel_diameter_mm;
    float wheelbase_mm;
    int steps_per_revolution;
    int pen_up_angle, pen_down_angle;
    
    // Current state
    RobotState state;
    float current_x, current_y, current_angle;  // Position in mm and radians
    bool pen_is_down;
    
    // Movement execution
    float target_x, target_y;
    bool has_target;
    float movement_speed_mms;
    
    // Step tracking for position estimation
    long left_steps_total, right_steps_total;
    long left_steps_at_start, right_steps_at_start;
    
public:
    // === INITIALIZATION ===
    void begin(const RobotConfig& config);
    
    // === HIGH-LEVEL MOVEMENT COMMANDS ===
    bool moveTo(float x, float y, float speed_mms = 15.0);    // Returns false if busy
    bool drawTo(float x, float y, float speed_mms = 10.0);    // Returns false if busy
    bool moveBy(float dx, float dy, float speed_mms = 15.0);
    bool drawBy(float dx, float dy, float speed_mms = 10.0);
    
    // === ROTATION COMMANDS ===
    bool turnTo(float angle_radians, float speed_rad_s = 0.5);
    bool turnBy(float delta_angle, float speed_rad_s = 0.5);
    
    // === PEN CONTROL ===
    void penUp();
    void penDown();
    bool isPenDown() const;
    
    // === STATE QUERIES ===
    RobotState getState() const;
    Position getPosition() const;           // {x, y, angle}
    bool isBusy() const;
    bool hasError() const;
    
    // === CONTROL ===
    void emergencyStop();
    void clearError();
    void resetPosition(float x = 0, float y = 0, float angle = 0);
    
    // === UPDATE FUNCTION ===
    void update();                          // Call every loop iteration
    
private:
    // Movement execution
    void executeMovement();
    void updatePositionEstimate();
    bool isAtTarget();
    
    // Kinematics
    void calculateSteps(float distance_mm, float angle_diff, int& left_steps, int& right_steps);
    void stepsToMovement(int left_steps, int right_steps, float& distance, float& angle_change);
    
    // Safety and limits
    bool isValidPosition(float x, float y);
    void setState(RobotState new_state);
};
```

**Implementation Status**: 🔴 Not started  
**Dependencies**: StepperDriver, ServoDriver  
**Testing**: Kinematics math, state machine logic, movement coordination

---

## Layer 3: Communication Interface

### CommandProcessor
**Purpose**: ESP32↔Nano communication and command execution  
**File**: `src/communication/CommandProcessor.h/cpp`

```cpp
struct Command {
    enum Type {
        MOVE_TO, DRAW_TO, MOVE_BY, DRAW_BY,
        TURN_TO, TURN_BY,
        PEN_UP, PEN_DOWN,
        GET_STATUS, STOP, RESET
    };
    
    uint32_t id;                // Unique command ID
    Type type;
    float x, y;                 // Coordinates (mm)
    float angle;                // Angle (radians)
    float speed;                // Speed (mm/s or rad/s)
    
    // Parsing from JSON
    static bool parseFromJson(const String& json, Command& cmd);
    String toJsonResponse(bool success, const String& error = "");
};

struct Status {
    uint32_t ack_id;           // Last successfully executed command ID
    float x, y, angle;         // Current position
    bool pen_down;
    bool busy;
    String error;              // Empty if no error
    
    String toJson() const;
};

class CommandProcessor {
private:
    TerraPenRobot& robot;
    
    // Communication state
    String input_buffer;
    uint32_t last_executed_id;
    unsigned long last_status_time;
    static const unsigned long STATUS_INTERVAL_MS = 100;  // 10Hz status updates
    
    // Command queue (for future expansion)
    static const int MAX_QUEUE_SIZE = 5;
    Command command_queue[MAX_QUEUE_SIZE];
    int queue_head, queue_tail, queue_count;
    
public:
    CommandProcessor(TerraPenRobot& robot_ref);
    
    // === COMMUNICATION ===
    void handleSerialInput();    // Process incoming serial data
    void sendStatusUpdate();     // Send periodic status
    void sendResponse(const Command& cmd, bool success, const String& error = "");
    
    // === COMMAND EXECUTION ===
    bool executeCommand(const Command& cmd);
    bool queueCommand(const Command& cmd);   // For future: command queuing
    
    // === UPDATE FUNCTION ===
    void update();               // Call every loop iteration
    
private:
    // JSON processing
    void processJsonCommand(const String& json);
    
    // Command execution helpers
    bool validateCommand(const Command& cmd);
    String getCommandError(const Command& cmd);
};
```

**Implementation Status**: 🔴 Not started  
**Dependencies**: TerraPenRobot, ArduinoJson library  
**Testing**: JSON parsing, command validation, communication protocol

---

## Supporting Data Structures

### RobotConfig
**Purpose**: Hardware configuration and parameters  
**File**: `src/RobotConfig.h`

```cpp
struct RobotConfig {
    // Physical parameters
    float wheel_diameter_mm;
    float wheelbase_mm;
    int steps_per_revolution;
    
    // Pin assignments
    int left_motor_pins[4];     // IN1, IN2, IN3, IN4
    int right_motor_pins[4];    // IN1, IN2, IN3, IN4
    int servo_pin;
    
    // Servo configuration
    int pen_up_angle;
    int pen_down_angle;
    
    // Performance limits
    float max_speed_mms;
    float max_angular_speed_rad_s;
    
    // Workspace limits (safety)
    float min_x, max_x;
    float min_y, max_y;
    
    // Default constructor
    RobotConfig() :
        wheel_diameter_mm(25.0),
        wheelbase_mm(30.0),
        steps_per_revolution(2048),
        servo_pin(9),
        pen_up_angle(90),
        pen_down_angle(0),
        max_speed_mms(50.0),
        max_angular_speed_rad_s(1.0),
        min_x(-100), max_x(100),
        min_y(-100), max_y(100)
    {
        // Default pin assignments
        left_motor_pins[0] = 2;   left_motor_pins[1] = 3;
        left_motor_pins[2] = 4;   left_motor_pins[3] = 5;
        right_motor_pins[0] = 6;  right_motor_pins[1] = 7;
        right_motor_pins[2] = 8;  right_motor_pins[3] = 9;
    }
};
```

**Implementation Status**: 🔴 Not started

---

### Position
**Purpose**: Robot position and orientation  
**File**: `src/Position.h`

```cpp
struct Position {
    float x, y;        // Coordinates in mm
    float angle;       // Orientation in radians
    
    Position(float x = 0, float y = 0, float angle = 0) : x(x), y(y), angle(angle) {}
    
    float distanceTo(const Position& other) const;
    float angleTo(const Position& other) const;
    Position offsetBy(float dx, float dy) const;
    
    String toString() const;
};
```

**Implementation Status**: 🔴 Not started

---

## Main Application

### Main Loop Structure
**File**: `src/main.cpp`

```cpp
#include "robot/TerraPenRobot.h"
#include "communication/CommandProcessor.h"
#include "RobotConfig.h"

// Global objects
RobotConfig config;
TerraPenRobot robot;
CommandProcessor comm(robot);

void setup() {
    Serial.begin(115200);
    
    // Initialize robot with configuration
    robot.begin(config);
    
    // Send ready signal
    Serial.println("{\"status\":\"ready\"}");
}

void loop() {
    // Handle communication (receives commands, sends status)
    comm.update();
    
    // Update robot state (executes movements)
    robot.update();
    
    // Small delay for stability
    delay(1);
}
```

**Implementation Status**: 🔴 Not started  
**Dependencies**: All layers  
**Testing**: End-to-end integration, communication flow

---

## Implementation Plan (Revised)

### Phase 1: Hardware Drivers (Week 1-2)
- [ ] **StepperDriver**: Basic stepping with timing
- [ ] **ServoDriver**: Servo control and smooth movement
- [ ] **Hardware validation**: Test with real motors
- [ ] **Unit tests**: Driver logic and state management

### Phase 2: Robot Control (Week 2-3)
- [ ] **TerraPenRobot**: Core robot class with state machine
- [ ] **Kinematics**: Forward/inverse differential drive math
- [ ] **Movement primitives**: moveTo, drawTo, pen control
- [ ] **Integration tests**: Complete robot movements

### Phase 3: Communication Layer (Week 3-4)
- [ ] **CommandProcessor**: JSON protocol implementation
- [ ] **Command execution**: Bridge between commands and robot
- [ ] **Status reporting**: Real-time robot state updates
- [ ] **Error handling**: Timeouts, invalid commands, emergency stop

### Phase 4: ESP32 Controller (Week 4-5)
- [ ] **ESP32 firmware**: Command queuing and WiFi interface
- [ ] **UART communication**: Reliable ESP32↔Nano protocol
- [ ] **Multi-robot support**: Device identification and coordination
- [ ] **Studio interface**: Basic PC control application

### Phase 5: Advanced Features (Week 5-6)
- [ ] **Trajectory planning**: Smooth acceleration profiles
- [ ] **Path optimization**: Minimize pen lifts, optimize travel
- [ ] **Calibration routines**: Auto-tuning and accuracy improvement
- [ ] **Studio enhancements**: Path visualization and simulation

### Phase 6: Multi-Robot System (Week 6+)
- [ ] **Swarm coordination**: Multiple robots working together
- [ ] **Collision avoidance**: Basic spatial awareness
- [ ] **Distributed drawing**: Large artwork coordination
- [ ] **Advanced simulation**: Physics-based robot modeling

---

## Design Patterns and Standards

### State Machine Pattern
```cpp
// Robot state transitions
IDLE → MOVING → IDLE
IDLE → DRAWING → IDLE  
ANY_STATE → EMERGENCY_STOP → IDLE (after clearError())
```

### Command Pattern
```cpp
// All robot actions encapsulated as commands
struct Command {
    virtual bool execute(TerraPenRobot& robot) = 0;
    virtual bool isComplete() = 0;
    virtual void abort() = 0;
};
```

### Observer Pattern (Future)
```cpp
// For status updates and event handling
class RobotObserver {
    virtual void onPositionChanged(Position pos) = 0;
    virtual void onStateChanged(RobotState state) = 0;
    virtual void onError(String error) = 0;
};
```

### Safety and Error Handling
- **Workspace limits**: Prevent robot from moving outside safe area
- **Emergency stop**: Immediate halt of all movement
- **Timeout protection**: Commands must complete within reasonable time
- **Graceful degradation**: System continues operating despite non-critical errors

---

## File Structure (Revised)

```
src/
├── hardware/
│   ├── StepperDriver.h/cpp
│   └── ServoDriver.h/cpp
├── robot/
│   └── TerraPenRobot.h/cpp
├── communication/
│   └── CommandProcessor.h/cpp
├── RobotConfig.h
├── Position.h
└── main.cpp

test/
├── test_drivers/
├── test_kinematics/
└── test_communication/

examples/
├── basic_movement/
├── drawing_patterns/
└── communication_test/
```

---

## ESP32 Controller Architecture

### ESP32 Responsibilities
- **Command queuing**: Buffer commands from Studio
- **Trajectory planning**: Generate smooth motion profiles
- **Multi-robot coordination**: Manage multiple Nano devices
- **WiFi interface**: Communicate with Studio application
- **Status monitoring**: Health checks and error reporting

### ESP32 Main Components
```cpp
class TerraController {
    WiFiServer studio_interface;
    MultipleSerial nano_interfaces;  // Support multiple robots
    CommandQueue command_buffer;
    TrajectoryPlanner motion_planner;
    
    void handleStudioCommands();
    void manageNanoDevices();
    void planAndExecuteTrajectories();
};
```

---

## Studio Application Architecture

### Python Studio Tool
```python
# Main components
class TerraPenStudio:
    def __init__(self):
        self.gui = PathPlanningGUI()
        self.simulator = RobotSimulator()
        self.communication = ESP32Interface()
        self.project_manager = DrawingProjectManager()
    
    # Core features
    def design_path(self):          # Interactive path creation
    def simulate_execution(self):    # Preview before sending to robot
    def send_to_robot(self):        # Execute on real hardware
    def monitor_progress(self):     # Real-time status display
```

### Studio Features
- **Path Editor**: Visual drawing tool with snap-to-grid, curves, shapes
- **Simulation**: Physics-based robot simulation for validation
- **Multi-robot Support**: Coordinate multiple robots for large artwork
- **Project Management**: Save/load drawing projects, version control
- **Real-time Monitoring**: Live position tracking, error reporting

---

## Communication Protocol Details

### Serial Protocol (ESP32 ↔ Nano)
- **Baud rate**: 115200
- **Format**: JSON messages, one per line
- **Flow control**: Command ID acknowledgment
- **Error handling**: Timeout and retry logic

### WiFi Protocol (Studio ↔ ESP32)
- **Transport**: WebSocket for real-time communication
- **Format**: JSON-RPC for structured command/response
- **Security**: WPA2 with optional API key authentication
- **Discovery**: mDNS for automatic device discovery

### Message Examples
```json
// Studio → ESP32 (High-level path)
{
  "method": "execute_path",
  "params": {
    "robot_id": "terra01",
    "segments": [
      {"type": "move_to", "x": 10, "y": 10, "speed": 15},
      {"type": "pen_down"},
      {"type": "draw_to", "x": 50, "y": 50, "speed": 10},
      {"type": "pen_up"}
    ]
  },
  "id": 1
}

// ESP32 → Nano (Individual commands)
{
  "id": 101,
  "cmd": "MOVE_TO",
  "x": 10,
  "y": 10,
  "speed": 15,
  "pen": false
}

// Nano → ESP32 (Status update)
{
  "ack": 101,
  "x": 9.8,
  "y": 9.9,
  "angle": 0.1,
  "pen": false,
  "busy": true,
  "error": null
}
```

---

## Testing Strategy (Revised)

### Unit Tests (Focus on Logic)
- **Driver state management**: Stepping sequences, servo positioning
- **Kinematics calculations**: Forward/inverse differential drive math
- **Command parsing**: JSON validation and error handling
- **State machine**: Valid transitions and error states

### Integration Tests (Focus on Coordination)
- **Robot movement**: Does moveTo() actually reach target?
- **Communication flow**: ESP32↔Nano command execution
- **Multi-layer coordination**: Do all layers work together?
- **Error propagation**: Do errors bubble up correctly?

### Hardware Validation (Focus on Real-world Performance)
- **Accuracy measurement**: Position error over time
- **Speed validation**: Actual vs commanded speeds
- **Repeatability**: Multiple runs to same target
- **Stress testing**: Continuous operation, error recovery

### System Tests (Focus on End-to-end Functionality)
- **Drawing quality**: Simple shapes, complex paths
- **Multi-robot coordination**: Two robots drawing together
- **Studio integration**: Complete workflow from design to execution
- **Performance benchmarks**: Response times, throughput

---

## Design Decisions & Rationale (Updated)

### Simplified Architecture
- **3 layers vs 5**: Reduces complexity while maintaining clear separation
- **Combined responsibilities**: Each layer handles a complete functional area
- **Standard patterns**: Aligns with proven embedded/robotics practices

### Communication-First Design
- **JSON protocol**: Human-readable, easy to debug, extensible
- **Command acknowledgment**: Reliable execution confirmation
- **Status streaming**: Real-time robot state for monitoring/debugging

### Non-blocking Execution
- **Cooperative multitasking**: Standard for Arduino/embedded systems
- **Single update() function**: Simple main loop, predictable timing
- **State-based processing**: Each component manages its own execution state

### Safety and Robustness
- **Emergency stop**: Always available, immediate response
- **Workspace limits**: Prevent physical damage or runaway
- **Error propagation**: Problems bubble up to higher layers
- **Graceful degradation**: System continues operating despite non-critical failures

---

*This document will be updated as implementation progresses.*

**Status Legend**:
- 🔴 Not started
- 🟡 In progress  
- 🟢 Complete
- ✅ Tested