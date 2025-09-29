/**
 * TerraPen Configuration Implementation
 */

#include "TerraPenConfig.h"
#include "ErrorSystem.h"

// Global configuration instance
TerraPenConfig g_config;

void TerraPenConfig::printConfiguration() {
    Serial.println("=== TerraPen Motion Control Configuration ===");
    Serial.print("Version: "); 
    Serial.print(TERRAPEN_VERSION_MAJOR); Serial.print(".");
    Serial.print(TERRAPEN_VERSION_MINOR); Serial.print(".");
    Serial.println(TERRAPEN_VERSION_PATCH);
    Serial.print("Build: "); Serial.print(TERRAPEN_BUILD_DATE); 
    Serial.print(" "); Serial.println(TERRAPEN_BUILD_TIME);
    Serial.print("Debug build: "); Serial.println(is_debug_build ? "YES" : "NO");
    Serial.println();
    
    // Hardware configuration
    Serial.println("--- Hardware Configuration ---");
    Serial.print("Left motor pins: ");
    for (int i = 0; i < 4; i++) {
        Serial.print(hardware.motor_l_pins[i]);
        if (i < 3) Serial.print(", ");
    }
    Serial.println();
    Serial.print("Right motor pins: ");
    for (int i = 0; i < 4; i++) {
        Serial.print(hardware.motor_r_pins[i]);
        if (i < 3) Serial.print(", ");
    }
    Serial.println();
    Serial.print("Servo pin: "); Serial.println(hardware.servo_pin);
    Serial.print("Wheel diameter: "); Serial.print(hardware.wheel_diameter_mm); Serial.println(" mm");
    Serial.print("Wheelbase: "); Serial.print(hardware.wheelbase_mm); Serial.println(" mm");
    Serial.print("Steps per revolution: "); Serial.println(hardware.steps_per_revolution);
    Serial.print("Step delay range: "); Serial.print(hardware.min_step_delay_us);
    Serial.print(" - "); Serial.print(hardware.max_step_delay_us); Serial.println(" μs");
    Serial.println();
    
    // Testing configuration
    Serial.println("--- Testing Configuration ---");
    Serial.print("POST enabled: "); Serial.println(testing.enable_post_on_startup ? "YES" : "NO");
    Serial.print("POST quick mode: "); Serial.println(testing.post_quick_mode ? "YES" : "NO");
    Serial.print("POST timeout: "); 
    Serial.print(testing.post_quick_mode ? testing.post_quick_timeout_ms : testing.post_full_timeout_ms); 
    Serial.println(" ms");
    Serial.print("Unit tests enabled: "); Serial.println(testing.enable_unit_tests ? "YES" : "NO");
    Serial.print("Test tags: ");
    if (testing.include_logic_tests) Serial.print("LOGIC ");
    if (testing.include_hardware_tests) Serial.print("HARDWARE ");
    if (testing.include_timing_tests) Serial.print("TIMING ");
    if (testing.include_stress_tests) Serial.print("STRESS ");
    Serial.println();
    Serial.println();
    
    // Performance configuration
    Serial.println("--- Performance Configuration ---");
    Serial.print("Baseline interval: "); Serial.print(performance.baseline_interval_ms); Serial.println(" ms");
    Serial.print("Storage interval: "); Serial.print(performance.storage_interval_ms); Serial.println(" ms");
    Serial.print("Anomaly interval: "); Serial.print(performance.anomaly_interval_ms); Serial.println(" ms");
    Serial.print("CPU anomaly threshold: "); Serial.print(performance.cpu_anomaly_percent); Serial.println("%");
    Serial.print("Timing anomaly threshold: "); Serial.print(performance.timing_anomaly_us); Serial.println(" us");
    Serial.println();
    
    // Communication configuration
    Serial.println("--- Communication Configuration ---");
    Serial.print("Debug serial: "); Serial.print(communication.debug_serial_baud); Serial.println(" baud");
    Serial.print("ESP32 upload: "); Serial.println(communication.enable_esp32_upload ? "YES" : "NO");
    if (communication.enable_esp32_upload) {
        Serial.print("ESP32 baud: "); Serial.print(communication.esp32_serial_baud); Serial.println(" baud");
        Serial.print("Upload batch size: "); Serial.println(communication.upload_batch_size);
        Serial.print("Heartbeat interval: "); Serial.print(communication.heartbeat_interval_ms); Serial.println(" ms");
    }
    Serial.println();
    
    // Storage configuration
    Serial.println("--- Storage Configuration ---");
    Serial.print("NVRAM logging: "); Serial.println(storage.enable_nvram_logging ? "YES" : "NO");
    Serial.print("Reserved space: "); Serial.print(storage.nvram_reserved_bytes); Serial.println(" bytes");
    Serial.print("Record limit: "); Serial.println(storage.nvram_record_limit);
    Serial.print("Data retention: "); Serial.print(storage.data_retention_hours); Serial.println(" hours");
    Serial.print("Auto cleanup: "); Serial.println(storage.auto_cleanup_uploaded ? "YES" : "NO");
    Serial.println();
    
    // Error configuration
    Serial.println("--- Error Configuration ---");
    Serial.print("Error logging: "); Serial.println(error_handling.enable_error_logging ? "YES" : "NO");
    Serial.print("Error upload: "); Serial.println(error_handling.enable_error_upload ? "YES" : "NO");
    Serial.print("History size: "); Serial.println(error_handling.error_history_size);
    Serial.print("Rate thresholds: "); Serial.print(error_handling.error_rate_warning);
    Serial.print(" / "); Serial.print(error_handling.error_rate_critical); Serial.println(" per minute");
    Serial.print("Auto recovery: "); Serial.println(error_handling.enable_auto_recovery ? "YES" : "NO");
    Serial.println();
    
    Serial.print("Configuration checksum: 0x"); Serial.println(config_checksum, HEX);
    Serial.println("==========================================");
}

bool TerraPenConfig::validateConfiguration() {
    bool valid = true;
    
    // Validate hardware configuration
    for (int i = 0; i < 4; i++) {
        if (!VALIDATE_PIN_RANGE(hardware.motor_l_pins[i]) || 
            !VALIDATE_PIN_RANGE(hardware.motor_r_pins[i])) {
            Serial.println("ERROR: Invalid motor pin configuration");
            valid = false;
        }
    }
    
    if (!VALIDATE_PIN_RANGE(hardware.servo_pin)) {
        Serial.println("ERROR: Invalid servo pin configuration");
        valid = false;
    }
    
    if (!VALIDATE_TIMING(hardware.min_step_delay_us) ||
        !VALIDATE_TIMING(hardware.max_step_delay_us) ||
        hardware.min_step_delay_us >= hardware.max_step_delay_us) {
        Serial.println("ERROR: Invalid step timing configuration");
        valid = false;
    }
    
    // Validate performance thresholds
    if (!VALIDATE_PERCENTAGE(performance.cpu_anomaly_percent) ||
        performance.timing_anomaly_us == 0 ||
        performance.frequency_anomaly_hz == 0) {
        Serial.println("ERROR: Invalid performance threshold configuration");
        valid = false;
    }
    
    // Validate timeout values
    if (testing.post_quick_timeout_ms >= testing.post_full_timeout_ms) {
        Serial.println("ERROR: Quick POST timeout must be less than full POST timeout");
        valid = false;
    }
    
    // Validate storage configuration
    if (storage.nvram_reserved_bytes > 512) {  // Don't reserve more than half of EEPROM
        Serial.println("ERROR: Too much NVRAM space reserved");
        valid = false;
    }
    
    // Check for pin conflicts
    bool pin_used[20] = {false};  // Arduino Nano has pins 0-19
    
    // Mark motor pins as used
    for (int i = 0; i < 4; i++) {
        if (pin_used[hardware.motor_l_pins[i]] || pin_used[hardware.motor_r_pins[i]]) {
            Serial.println("ERROR: Pin conflict detected in motor configuration");
            valid = false;
        }
        pin_used[hardware.motor_l_pins[i]] = true;
        pin_used[hardware.motor_r_pins[i]] = true;
    }
    
    // Check servo pin
    if (pin_used[hardware.servo_pin]) {
        Serial.println("ERROR: Servo pin conflicts with motor pins");
        valid = false;
    }
    
    return valid;
}

void TerraPenConfig::resetToDefaults() {
    // Reset all configurations to default values
    hardware = HardwareConfig();
    testing = TestingConfig();
    performance = PerformanceConfig();
    communication = CommunicationConfig();
    storage = StorageConfig();
    error_handling = ErrorConfig();
    
    config_version = 1;
    config_checksum = calculateChecksum();
}

uint32_t TerraPenConfig::calculateChecksum() {
    uint32_t checksum = 0;
    uint8_t* data = (uint8_t*)this;
    
    // Calculate checksum for everything except the checksum field itself
    for (size_t i = 0; i < sizeof(TerraPenConfig) - sizeof(uint32_t); i++) {
        checksum += data[i];
    }
    
    return checksum;
}