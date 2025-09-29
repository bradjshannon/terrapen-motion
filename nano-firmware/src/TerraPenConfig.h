/**
 * TerraPen Motion Control - Master Configuration
 * 
 * Centralized configuration for all project components including:
 * - Hardware settings (pins, timing, limits)
 * - Testing framework settings (timeouts, verbosity, tags)
 * - Performance monitoring settings (intervals, thresholds)
 * - Communication settings (ESP32, serial, protocols)
 * - Error handling settings (reporting, thresholds)
 * - NVRAM/storage settings (buffer sizes, upload intervals)
 */

#ifndef TERRAPEN_CONFIG_H
#define TERRAPEN_CONFIG_H

#include <Arduino.h>

// === PROJECT METADATA ===
#define TERRAPEN_VERSION_MAJOR 1
#define TERRAPEN_VERSION_MINOR 5
#define TERRAPEN_VERSION_PATCH 0
#define TERRAPEN_BUILD_DATE __DATE__
#define TERRAPEN_BUILD_TIME __TIME__

// === HARDWARE CONFIGURATION ===
struct HardwareConfig {
    // === STEPPER MOTOR PINS ===
    // Left motor (28BYJ-48 with ULN2803A)
    uint8_t motor_l_pins[4] = {2, 3, 4, 5};     // IN1, IN2, IN3, IN4
    
    // Right motor (28BYJ-48 with ULN2803A)  
    uint8_t motor_r_pins[4] = {6, 7, 8, 9};     // IN1, IN2, IN3, IN4
    
    // === SERVO CONFIGURATION ===
    uint8_t servo_pin = 10;                      // Pen servo control pin
    uint16_t servo_pen_up_angle = 90;            // Degrees for pen up position
    uint16_t servo_pen_down_angle = 45;          // Degrees for pen down position
    uint16_t servo_move_speed_ms = 500;          // Time for pen up/down movement
    
    // === PHYSICAL PARAMETERS ===
    float wheel_diameter_mm = 25.0f;             // Wheel diameter in millimeters
    float wheelbase_mm = 30.0f;                  // Distance between wheel centers
    uint16_t steps_per_revolution = 2048;        // Steps for 360° rotation (28BYJ-48)
    
    // === MOTOR TIMING ===
    uint16_t step_delay_us = 1000;               // Microseconds between steps (max speed)
    uint16_t min_step_delay_us = 600;            // Minimum step delay (hardware limit)
    uint16_t max_step_delay_us = 10000;          // Maximum step delay (slowest speed)
    uint16_t acceleration_steps = 50;            // Steps to reach full speed
    
    // === SAFETY LIMITS ===
    uint32_t max_continuous_steps = 50000;       // Maximum steps before mandatory pause
    uint16_t emergency_stop_timeout_ms = 100;    // Max time for emergency stop response
    uint16_t movement_timeout_ms = 30000;        // Max time for any single movement
    
    // === WORKSPACE BOUNDARIES (Phase 2) ===
    float workspace_min_x = -100.0f;            // Minimum X coordinate in mm
    float workspace_max_x = 100.0f;             // Maximum X coordinate in mm
    float workspace_min_y = -100.0f;            // Minimum Y coordinate in mm
    float workspace_max_y = 100.0f;             // Maximum Y coordinate in mm
    
    // === POWER MANAGEMENT ===
    uint8_t motor_hold_current_percent = 30;     // Holding current as % of full
    uint16_t motor_sleep_timeout_ms = 5000;      // Time before motors sleep
    bool enable_power_saving = true;             // Enable automatic power management
};

// === TESTING CONFIGURATION ===
struct TestingConfig {
    // === POST (Power-On Self Test) SETTINGS ===
    bool enable_post_on_startup = true;          // Run POST automatically on boot
    bool post_quick_mode = true;                 // Use quick POST by default
    uint16_t post_quick_timeout_ms = 2000;       // Quick POST timeout (2 seconds)
    uint16_t post_full_timeout_ms = 8000;        // Full POST timeout (8 seconds - optimized)
    bool post_verbose_output = false;            // Detailed POST output
    bool post_stop_on_failure = true;            // Stop robot operation if POST fails
    
    // === UNIT TESTING SETTINGS ===
    bool enable_unit_tests = true;               // Include unit tests in build
    bool auto_run_tests_on_debug = false;       // Auto-run tests in debug builds
    uint16_t test_timeout_ms = 5000;             // Individual test timeout
    bool test_verbose_assertions = false;        // Print all assertion details
    
    // === TEST TAGGING ===
    bool include_logic_tests = true;             // Include TEST_TAG_LOGIC tests
    bool include_hardware_tests = true;          // Include TEST_TAG_HARDWARE tests
    bool include_timing_tests = false;           // Include TEST_TAG_TIMING tests (slow)
    bool include_stress_tests = false;           // Include TEST_TAG_STRESS tests
    
    // === TEST AUTOMATION ===
    bool enable_git_hooks = true;                // Enable pre-commit testing
    bool enable_ci_compilation = true;           // Enable CI compilation checks
    uint8_t test_retry_count = 2;                // Retries for flaky tests
};

// === ADAPTIVE PERFORMANCE MONITORING ===
struct PerformanceConfig {
    // === ADAPTIVE LOGGING STRATEGY ===
    // Normal operation: Log broad patterns every 10 minutes (minimal EEPROM wear)
    // Anomaly detection: Switch to detailed 5-second logging when issues detected
    uint32_t baseline_interval_ms = 600000;      // Baseline: every 10 minutes
    uint16_t anomaly_interval_ms = 5000;         // Anomaly: every 5 seconds
    uint32_t storage_interval_ms = 600000;       // EEPROM write: every 10 minutes normally
    
    // === ANOMALY DETECTION THRESHOLDS ===
    // Trigger detailed logging when performance deviates from normal patterns
    uint16_t timing_anomaly_us = 5000;           // Loop time > 5ms triggers anomaly mode
    uint16_t frequency_anomaly_hz = 90;          // Frequency < 90Hz triggers anomaly mode
    uint8_t cpu_anomaly_percent = 80;            // CPU > 80% triggers anomaly mode
    uint16_t step_deviation_us = 50;             // Step timing variance > 50us
    
    // === ADAPTIVE BEHAVIOR ===
    uint16_t anomaly_duration_ms = 30000;        // Stay in anomaly mode for 30 seconds
    uint8_t anomaly_cooldown_cycles = 3;         // Require 3 normal readings to exit anomaly mode
    uint8_t baseline_sample_window = 3;          // Small window for baseline (broad patterns)
    uint8_t anomaly_sample_window = 15;          // Larger window during anomalies (detail)
    
    // === SMART MONITORING FEATURES ===
    bool enable_adaptive_logging = true;         // Enable smart anomaly detection
    bool enable_memory_monitoring = false;       // DISABLED - too expensive on AVR
    bool enable_step_timing_analysis = true;     // Monitor stepper performance patterns
    bool enable_pattern_detection = true;        // Detect performance trends
    
    // === EEPROM WEAR PROTECTION ===
    uint8_t max_anomaly_records_per_hour = 12;   // Limit anomaly writes (5-min max frequency)
    bool compress_baseline_data = true;          // Store only essential baseline metrics
};

// === COMMUNICATION CONFIGURATION ===
struct CommunicationConfig {
    // === SERIAL COMMUNICATION ===
    uint32_t debug_serial_baud = 115200;         // Debug serial baud rate
    uint32_t esp32_serial_baud = 115200;         // ESP32 communication baud rate
    uint16_t serial_timeout_ms = 1000;           // Serial read timeout
    
    // === ESP32 UPLOAD ===
    bool enable_esp32_upload = false;            // Enable ESP32 data upload
    uint16_t upload_retry_interval_ms = 30000;   // Retry interval for failed uploads
    uint8_t upload_max_retries = 3;              // Max upload retry attempts
    uint16_t upload_batch_size = 10;             // Records per upload batch
    uint32_t heartbeat_interval_ms = 60000;      // ESP32 heartbeat interval
    
    // === PROTOCOL SETTINGS ===
    uint16_t command_timeout_ms = 2000;          // Command response timeout
    uint8_t protocol_version = 1;                // Communication protocol version
    bool enable_checksums = true;                // Enable data integrity checks
};

// === STORAGE CONFIGURATION ===
struct StorageConfig {
    // === NVRAM/EEPROM SETTINGS (OPTIMIZED FOR REAL-WORLD USAGE) ===
    bool enable_nvram_logging = true;            // Enable NVRAM performance logging
    uint16_t nvram_reserved_bytes = 256;         // Bytes reserved for future use
    uint16_t nvram_record_limit = 46;            // Max records (based on EEPROM capacity)
    
    // === DATA RETENTION (OPTIMIZED FOR 24+ HOUR COVERAGE) ===
    uint32_t data_retention_hours = 24;          // Keep data for 24 hours (realistic for robot)
    bool auto_cleanup_uploaded = true;           // Auto-delete confirmed uploads
    uint32_t cleanup_interval_ms = 600000;       // Cleanup every 10 minutes (was 5)
    
    // === STORAGE STRATEGY (OPTIMIZED) ===
    uint32_t storage_interval_ms = 1800000;      // Store every 30 minutes (was 60s)
    bool adaptive_storage = true;                // Store more frequently during errors
    bool compress_data = true;                   // Use compressed format when possible
    uint8_t error_boost_factor = 6;              // Store 6x more often during errors (every 5 min)
    
    // === UPLOAD OPTIMIZATION ===
    uint8_t upload_batch_size = 5;               // Upload 5 records at once (was 10)
    bool prioritize_error_records = true;        // Upload error records first
    uint16_t upload_window_hours = 2;            // Try to upload within 2 hours
    
    // === WEAR LEVELING ===
    uint32_t max_write_cycles = 90000;           // Conservative EEPROM limit
    bool enable_wear_monitoring = true;          // Monitor EEPROM wear
    uint8_t wear_warning_percent = 80;           // Wear level warning threshold
};

// === ERROR HANDLING CONFIGURATION ===
struct ErrorConfig {
    // === ERROR REPORTING ===
    bool enable_error_logging = true;            // Log errors to NVRAM
    bool enable_error_upload = false;            // Upload errors to ESP32
    uint8_t error_history_size = 16;             // Number of recent errors to keep
    
    // === ERROR THRESHOLDS ===
    uint8_t error_rate_warning = 10;             // Errors per minute warning
    uint8_t error_rate_critical = 20;            // Errors per minute critical
    bool halt_on_critical_error = true;          // Stop robot on critical errors
    
    // === ERROR RECOVERY ===
    bool enable_auto_recovery = true;            // Attempt automatic error recovery
    uint8_t recovery_retry_count = 3;            // Recovery attempt limit
    uint16_t recovery_delay_ms = 1000;           // Delay between recovery attempts
};

// === MASTER CONFIGURATION STRUCTURE ===
struct TerraPenConfig {
    HardwareConfig hardware;
    TestingConfig testing;
    PerformanceConfig performance;
    CommunicationConfig communication;
    StorageConfig storage;
    ErrorConfig error_handling;
    
    // === CONFIGURATION METADATA ===
    uint16_t config_version = 1;                 // Configuration format version
    uint32_t config_checksum = 0;                // Configuration integrity check
    
    // === HELPER METHODS ===
    void printConfiguration();
    bool validateConfiguration();
    void resetToDefaults();
    uint32_t calculateChecksum();
    
    // === CONDITIONAL COMPILATION HELPERS ===
    #if defined(DEBUG) || defined(_DEBUG)
    static constexpr bool is_debug_build = true;
    #else
    static constexpr bool is_debug_build = false;
    #endif
    
    #ifdef ENABLE_HARDWARE_TESTS
    static constexpr bool hardware_tests_enabled = true;
    #else
    static constexpr bool hardware_tests_enabled = false;
    #endif
};

// === GLOBAL CONFIGURATION INSTANCE ===
extern TerraPenConfig g_config;

// === CONFIGURATION MACROS ===
#define HARDWARE_CONFIG g_config.hardware
#define TESTING_CONFIG g_config.testing
#define PERFORMANCE_CONFIG g_config.performance
#define COMMUNICATION_CONFIG g_config.communication
#define STORAGE_CONFIG g_config.storage
#define ERROR_CONFIG g_config.error_handling

// === BUILD-TIME CONFIGURATION OVERRIDES ===
// These can be set via compiler flags to override defaults

#ifndef TERRAPEN_ENABLE_POST
#define TERRAPEN_ENABLE_POST true
#endif

#ifndef TERRAPEN_ENABLE_ESP32
#define TERRAPEN_ENABLE_ESP32 false
#endif

#ifndef TERRAPEN_DEBUG_VERBOSE
#define TERRAPEN_DEBUG_VERBOSE false
#endif

#ifndef TERRAPEN_MAX_SPEED_SPS
#define TERRAPEN_MAX_SPEED_SPS 300
#endif

// === CONFIGURATION VALIDATION MACROS ===
#define VALIDATE_PIN_RANGE(pin) ((pin) >= 0 && (pin) <= 19)
#define VALIDATE_TIMING(us) ((us) >= 100 && (us) <= 100000)
#define VALIDATE_PERCENTAGE(pct) ((pct) >= 0 && (pct) <= 100)

#endif // TERRAPEN_CONFIG_H