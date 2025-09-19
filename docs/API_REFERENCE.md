# TerraPen Motion Control API Reference

**Version**: 2.0.0  
**Date**: September 19, 2025

## Overview

Complete API reference for the TerraPen Motion Control library, including all classes, functions, configuration options, and testing framework components. Now includes Phase 2 coordinate-based movement system with differential drive kinematics.

## Core Classes

### TerraPenRobot
Main robot control class with state machine and coordinated movement.

#### Constructor
```cpp
TerraPenRobot();
```

#### Initialization
```cpp
bool begin();  // Uses g_config.hardware
void reset();
```

#### Coordinate-Based Movement (Phase 2)
```cpp
bool moveTo(float x, float y, float speed_mms = 15.0);     // Move to coordinates with pen up
bool drawTo(float x, float y, float speed_mms = 10.0);     // Draw line to coordinates with pen down
bool moveBy(float dx, float dy, float speed_mms = 15.0);   // Move relative to current position
bool drawBy(float dx, float dy, float speed_mms = 10.0);   // Draw relative to current position
```

#### Rotation Control (Phase 2)
```cpp
bool turnTo(float angle_radians, float speed_rad_s = 0.5); // Turn to absolute angle
bool turnBy(float delta_angle, float speed_rad_s = 0.5);   // Turn by relative angle
```

#### Step-Based Movement (Phase 1.5)
```cpp
bool moveForward(int steps);
bool moveBackward(int steps); 
bool turnLeft(int steps);
bool turnRight(int steps);
void emergencyStop();
```

#### Pen Control
```cpp
void penUp();
void penDown();
bool isPenDown();
```

#### State Management
```cpp
RobotState getState();
bool isBusy();
bool isMoving();
bool hasError();
```

#### Position Tracking (Phase 2)
```cpp
Position getCurrentPosition();               // Get current position and orientation
void resetPosition(float x = 0, float y = 0, float angle = 0); // Reset position tracking
bool isAtTarget();                          // Check if at target position
bool isValidPosition(float x, float y);     // Check workspace boundaries
```

#### Update Loop
```cpp
void update();  // Call in main loop for non-blocking operation
```

### StepperDriver
Individual stepper motor control with precise timing.

#### Constructor
```cpp
StepperDriver();
```

#### Initialization
```cpp
void begin(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4);
void setSpeed(float steps_per_second);
```

#### Movement
```cpp
bool stepForward();
bool stepBackward();
void stop();
void hold();
void release();
```

#### Status
```cpp
bool isReady();
bool isHolding();
uint32_t getTotalSteps();
```

### ServoDriver
Smooth servo control for pen up/down mechanism.

#### Constructor
```cpp
ServoDriver();
```

#### Initialization
```cpp
bool begin(uint8_t pin, uint16_t initial_angle = 90);
void attach(uint8_t pin);
void detach();
```

#### Movement
```cpp
void setAngle(uint16_t angle);
void sweepTo(uint16_t target_angle, uint16_t duration_ms);
void stop();
```

#### Status
```cpp
bool isAttached();
bool isMoving();
uint16_t getCurrentAngle();
```

#### Update
```cpp
void update();  // Call in main loop for smooth movement
```

## Configuration System

### TerraPenConfig (Global: g_config)
Centralized configuration for all project components.

#### Hardware Configuration
```cpp
HardwareConfig hardware = {
    .motor_l_pins = {2, 3, 4, 5},        // Left motor pins
    .motor_r_pins = {6, 7, 8, 9},        // Right motor pins
    .servo_pin = 10,                     // Servo control pin
    .wheel_diameter_mm = 25.0f,          // Wheel diameter
    .wheelbase_mm = 30.0f,               // Distance between wheels
    .steps_per_revolution = 2048,        // Stepper steps per revolution
    .step_delay_us = 1000,               // Step timing
    .min_step_delay_us = 600,            // Minimum step delay
    .max_step_delay_us = 10000           // Maximum step delay
};
```

#### Testing Configuration
```cpp
TestingConfig testing = {
    .enable_post_on_startup = true,      // Run POST on boot
    .post_quick_mode = true,             // Use quick POST
    .post_quick_timeout_ms = 2000,       // Quick POST timeout
    .post_full_timeout_ms = 30000,       // Full POST timeout
    .enable_unit_tests = true,           // Include unit tests
    .include_logic_tests = true,         // Include logic tests
    .include_hardware_tests = true,      // Include hardware tests
    .include_timing_tests = false        // Include timing tests
};
```

#### Performance Configuration
```cpp
PerformanceConfig performance = {
    .sample_interval_ms = 100,           // Monitoring sample rate
    .storage_interval_ms = 10000,        // NVRAM storage interval
    .cpu_warning_threshold = 80,         // CPU usage warning (%)
    .cpu_critical_threshold = 95,        // CPU usage critical (%)
    .memory_warning_bytes = 200,         // Low memory warning
    .memory_critical_bytes = 50          // Critical memory level
};
```

### RobotConfig (Legacy Compatibility)
Hardware configuration wrapper for backward compatibility.

```cpp
RobotConfig config;  // Loads from g_config.hardware
config.wheel_diameter_mm = 30.0;  // Override if needed
```

## Testing Framework

### Test Macros
```cpp
// Test case definitions
TEST_CASE_LOGIC(name)      // Logic/algorithm tests
TEST_CASE_HARDWARE(name)   // Hardware interaction tests
TEST_CASE_POST(name)       // Power-on self test
TEST_CASE_TIMING(name)     // Timing validation tests
TEST_CASE_STRESS(name)     // Stress/load tests

// Assertion macros
TEST_ASSERT_TRUE(condition)
TEST_ASSERT_FALSE(condition) 
TEST_ASSERT_EQUAL(expected, actual)
TEST_ASSERT_NOT_EQUAL(not_expected, actual)
TEST_ASSERT_GREATER(value, minimum)
TEST_ASSERT_LESS(value, maximum)
```

### POST (Power-On Self Test)
```cpp
// Quick POST (2 seconds)
PostResults runQuickPost();

// Full POST (30 seconds)
PostResults runFullPost();

// POST configuration
PostConfig config = {
    .run_hardware_tests = true,
    .run_safety_tests = true,
    .run_timing_tests = false,
    .verbose_output = false,
    .timeout_ms = 10000
};

// POST results
struct PostResults {
    bool passed;
    int tests_run;
    int tests_passed;
    int tests_failed;
    unsigned long duration_ms;
    String failure_summary;
    ErrorCode critical_error;
};
```

### Performance Monitoring
```cpp
// Global performance monitor
extern PerformanceMonitor performance_monitor;

// Initialize monitoring
performance_monitor.begin();

// Get current metrics
PerformanceMetrics metrics = performance_monitor.getMetrics();

// Performance metrics structure
struct PerformanceMetrics {
    float cpu_usage_percent;         // 0-100%
    uint16_t free_memory_bytes;      // Available RAM
    uint32_t avg_loop_time_us;       // Average loop time
    uint32_t max_loop_time_us;       // Peak loop time
    uint16_t current_step_rate;      // Steps per second
    uint8_t error_count_since_last;  // Recent error count
    bool cpu_overload_warning;       // CPU usage warning
    bool memory_low_warning;         // Memory warning
};
```

## Storage & NVRAM

### NVRAMManager (Global: g_nvram_manager)
Performance data storage with ESP32 upload capability.

#### Initialization
```cpp
g_nvram_manager.begin(&esp32_uploader);  // With uploader
g_nvram_manager.begin();                 // Without uploader
```

#### Data Storage
```cpp
bool storePerformanceData(const PerformanceMetrics& metrics);
bool hasUnuploadedData();
uint16_t getUnuploadedRecordCount();
uint16_t getAvailableSpace();
```

#### Upload Management
```cpp
void setUploader(ESP32Uploader* uploader);
bool uploadPendingData();
void confirmUploaded(uint32_t timestamp);
```

#### Maintenance
```cpp
void performMaintenance();
float getEEPROMWearLevel();
void printStorageStatus();
```

### Performance Record Structure
```cpp
struct PerformanceRecord {
    uint32_t timestamp;           // millis() when recorded
    uint16_t cpu_usage_percent;   // 0-10000 (0.01% resolution)
    uint16_t free_memory_bytes;   // Available RAM
    uint16_t loop_time_us;        // Loop execution time
    uint16_t step_rate_sps;       // Steps per second
    uint8_t error_count;          // Errors since last record
    uint8_t upload_status;        // Upload status
    uint16_t checksum;            // Data integrity
};
```

## Error Management

### Error System (Global: g_error_manager)
Structured error handling with context tracking.

#### Error Reporting Macros
```cpp
REPORT_ERROR(code, context, message)
CLEAR_ERROR()
HAS_ERROR()
```

#### Error Manager Functions
```cpp
bool hasError();
ErrorCode getCurrentErrorCode();
String getCurrentContext();
String getCurrentMessage();
void clearError();
void printErrorHistory();
```

#### Error Codes (Selection)
```cpp
// Hardware errors (1-20)
ERR_STEPPER_NOT_INITIALIZED = 1,
ERR_SERVO_ATTACH_FAILED = 2,
ERR_INVALID_PIN_CONFIG = 3,

// Movement errors (21-40)
ERR_INVALID_SPEED = 21,
ERR_MOVEMENT_TIMEOUT = 22,
ERR_EMERGENCY_STOP = 23,

// System errors (41-60)
ERR_MEMORY_ALLOCATION = 41,
ERR_EEPROM_WRITE_FAILED = 42,
ERR_COMMUNICATION_FAILED = 43
```

## Communication

### ESP32Uploader
Data upload to ESP32 via serial communication.

#### Initialization
```cpp
ESP32Uploader uploader;
uploader.begin(&Serial1, 115200);  // Hardware serial
uploader.begin(&Serial, 115200);   // Debug serial
```

#### Connection Management
```cpp
bool isConnected();
bool testConnection();
void handleHeartbeat();
```

#### Data Upload
```cpp
bool sendPerformanceData(const PerformanceRecord& record);
bool sendBulkData(const PerformanceRecord* records, uint16_t count);
bool sendErrorReport(uint8_t error_code, const String& context);
bool sendStatusUpdate(const String& status);
```

#### Configuration
```cpp
void setMaxRetries(uint8_t retries);
void setHeartbeatInterval(unsigned long interval_ms);
```

## Utility Functions

### Configuration Validation
```cpp
bool validateRobotConfig(const RobotConfig& config);
bool g_config.validateConfiguration();
void g_config.printConfiguration();
void g_config.resetToDefaults();
```

### Pin Validation Macros
```cpp
VALIDATE_PIN_RANGE(pin)     // Check if pin is valid (0-19)
VALIDATE_TIMING(us)         // Check if timing is valid (100-100000)
VALIDATE_PERCENTAGE(pct)    // Check if percentage is valid (0-100)
```

### Build-time Configuration
```cpp
// Compiler flags for build customization
#define TERRAPEN_ENABLE_POST true
#define TERRAPEN_ENABLE_ESP32 false
#define TERRAPEN_DEBUG_VERBOSE false
#define TERRAPEN_MAX_SPEED_SPS 300
```

## Constants and Enumerations

### Robot States
```cpp
enum RobotState {
    IDLE,              // Robot ready for commands
    MOVING,            // Robot executing movement
    ERROR,             // Robot in error state
    EMERGENCY_STOP     // Emergency stop activated
};
```

### Upload Status
```cpp
enum UploadStatus : uint8_t {
    UPLOAD_PENDING = 0,    // Not yet uploaded
    UPLOAD_SENT = 1,       // Uploaded but not confirmed
    UPLOAD_CONFIRMED = 2   // Upload confirmed by ESP32
};
```

### Test Tags
```cpp
#define TEST_TAG_LOGIC     0x01    // Algorithm/logic tests
#define TEST_TAG_HARDWARE  0x02    // Hardware interaction tests
#define TEST_TAG_POST      0x04    // Power-on self tests
#define TEST_TAG_TIMING    0x08    // Timing validation tests
#define TEST_TAG_STRESS    0x10    // Stress/load tests
```

## Example Usage Patterns

## Example Usage Patterns

### Phase 2 Coordinate-Based Movement (Recommended)
```cpp
#include <TerraPenMotionControl.h>

TerraPenRobot robot;

void setup() {
    Serial.begin(115200);
    
    // Initialize robot with global config
    robot.begin();
    
    // Set starting position (optional calibration)
    robot.resetPosition(0, 0, 0);
    
    Serial.println("Robot ready for coordinate movement");
}

void loop() {
    robot.update();  // Always call this
    
    if (!robot.isBusy()) {
        // Draw a simple square (20mm x 20mm)
        robot.moveTo(0, 0);       // Move to start (pen up automatically)
        robot.drawTo(20, 0);      // Draw right side (pen down automatically)
        robot.drawTo(20, 20);     // Draw top side
        robot.drawTo(0, 20);      // Draw left side
        robot.drawTo(0, 0);       // Draw bottom side
        
        // Move to new location and draw circle approximation
        robot.moveTo(40, 40);     // Move pen up to new location
        drawCircle(40, 40, 10);   // Draw 10mm radius circle
        
        // Print current position
        Position pos = robot.getCurrentPosition();
        Serial.print("Current position: ");
        pos.print();
        
        delay(5000);  // Wait before repeating
    }
}

void drawCircle(float center_x, float center_y, float radius) {
    const int segments = 12;  // 12-sided polygon approximation
    
    // Move to start point on circle
    robot.moveTo(center_x + radius, center_y);
    robot.penDown();
    
    // Draw circle segments
    for (int i = 1; i <= segments; i++) {
        float angle = (2 * PI * i) / segments;
        float x = center_x + radius * cos(angle);
        float y = center_y + radius * sin(angle);
        robot.drawTo(x, y);
    }
    
    robot.penUp();
}
```

### Phase 1.5 Step-Based Movement (Legacy Support)
```cpp
#include <TerraPenMotionControl.h>

TerraPenRobot robot;

void setup() {
    robot.begin();
}

void loop() {
    robot.update();  // Always call this
    
    if (!robot.isBusy()) {
        // Step-based movement (no coordinates)
        robot.moveForward(50);    // Move 50 steps forward
        robot.penDown();          // Lower pen
        robot.turnLeft(25);       // Turn left 25 steps
        robot.penUp();            // Raise pen
        
        delay(2000);
    }
}
```

### Coordinate System Conventions
The TerraPen coordinate system follows robotics conventions:
- **Origin (0,0)**: Robot's starting position (call `resetPosition()` to set)
- **X-axis**: Positive right, negative left (robot's perspective)
- **Y-axis**: Positive forward, negative backward (robot's perspective)  
- **Angle**: 0 = facing positive Y, positive rotation = counterclockwise
- **Units**: All distances in millimeters, angles in radians
- **Speed**: Linear speed in mm/s, angular speed in rad/s

### Workspace Boundaries
The robot automatically validates all coordinate movements against configured workspace boundaries:
```cpp
// Default workspace (configurable in g_config.hardware)
// X: -100mm to +100mm
// Y: -100mm to +100mm

bool valid = robot.isValidPosition(50, 75);    // Returns true - within bounds
bool invalid = robot.isValidPosition(150, 0);  // Returns false - outside bounds

// Attempting invalid movement will return false
bool success = robot.moveTo(150, 0);  // Returns false, robot doesn't move
```

### Error Handling Pattern
```cpp
void performRobotTask() {
    if (HAS_ERROR()) {
        Serial.println("Cannot perform task - system has error");
        return;
    }
    
    if (robot.isBusy()) {
        return;  // Try again later
    }
    
    // Validate target position
    if (!robot.isValidPosition(target_x, target_y)) {
        REPORT_ERROR(ERR_INVALID_POSITION, "performRobotTask", "Target outside workspace");
        return;
    }
    
    if (!robot.moveTo(target_x, target_y)) {
        REPORT_ERROR(ERR_MOVEMENT_FAILED, "performRobotTask", "Movement command failed");
        return;
    }
    
    // Task completed successfully
}
```

### Position Tracking and Accuracy
```cpp
void monitorPositionAccuracy() {
    // Get current estimated position
    Position current = robot.getCurrentPosition();
    
    // Move to known position
    robot.moveTo(50, 50);
    
    // Wait for completion
    while (robot.isBusy()) {
        robot.update();
        delay(10);
    }
    
    // Check final position
    Position final = robot.getCurrentPosition();
    float error = final.distanceTo(Position(50, 50));
    
    Serial.print("Position error: ");
    Serial.print(error, 2);
    Serial.println(" mm");
    
    if (error > 2.0) {
        Serial.println("Warning: Position accuracy degraded - consider recalibration");
        robot.resetPosition(50, 50, final.angle);  // Manual correction
    }
}
```

### Error Handling Pattern
```cpp
void performRobotTask() {
    if (HAS_ERROR()) {
        Serial.println("Cannot perform task - system has error");
        return;
    }
    
    if (robot.isBusy()) {
        return;  // Try again later
    }
    
    if (!robot.moveForward(100)) {
        REPORT_ERROR(ERR_MOVEMENT_FAILED, "performRobotTask", "Forward movement failed");
        return;
    }
    
    // Task completed successfully
}
```

### Performance Monitoring Pattern
```cpp
void monitorPerformance() {
    PerformanceMetrics metrics = performance_monitor.getMetrics();
    
    if (metrics.cpu_overload_warning) {
        Serial.println("Warning: High CPU usage");
        // Reduce operation frequency
    }
    
    if (metrics.memory_low_warning) {
        Serial.println("Warning: Low memory");
        // Free unused resources
    }
    
    // Store metrics to NVRAM every 10 seconds
    static unsigned long last_storage = 0;
    if (millis() - last_storage > 10000) {
        g_nvram_manager.storePerformanceData(metrics);
        last_storage = millis();
    }
}