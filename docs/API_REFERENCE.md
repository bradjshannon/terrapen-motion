# TerraPen Motion Control API Reference

**Version**: 1.6.0  
**Date**: September 19, 2025

## Overview

Complete API reference for the TerraPen Motion Control library, including all classes, functions, configuration options, and testing framework components.

## Core Classes

### TerraPenRobot
Main robot control class with state machine and coordinated movement.

#### Constructor
```cpp
TerraPenRobot();
```

#### Initialization
```cpp
bool begin(const RobotConfig& config);
void reset();
```

#### Movement Commands
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

### Complete Robot Setup
```cpp
#include <TerraPenMotionControl.h>

void setup() {
    Serial.begin(115200);
    
    // Configure system
    g_config.testing.enable_post_on_startup = true;
    g_config.performance.enable_cpu_monitoring = true;
    g_config.storage.enable_nvram_logging = true;
    
    // Run POST
    PostResults post = runQuickPost();
    if (!post.passed) {
        Serial.println("POST failed - halting");
        while(1);
    }
    
    // Initialize components
    performance_monitor.begin();
    g_nvram_manager.begin();
    
    // Initialize robot
    TerraPenRobot robot;
    RobotConfig config;
    robot.begin(config);
    
    Serial.println("System ready");
}

void loop() {
    robot.update();
    performance_monitor.update();
    g_nvram_manager.performMaintenance();
    
    // Robot operations...
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