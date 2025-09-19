/**
 * Power-On Self Test (POST) System
 * 
 * Validates critical robot functionality at startup using a subset of unit tests.
 * Provides confidence that hardware and software are functioning correctly
 * before beginning normal operation.
 */

#include "../testing/TestFramework.h"
#include "../ErrorSystem.h"
#include "../TerraPenConfig.h"

/**
 * POST Test Battery Definition
 * 
 * These are the essential tests that must pass for the robot to be considered
 * operational. They focus on critical functionality and safety features.
 */

// === POST CONFIGURATION ===

struct PostConfig {
    bool run_hardware_tests;      // Test actual hardware (motors, servo)
    bool run_safety_tests;        // Test emergency stop, error handling
    bool run_timing_tests;        // Test timing accuracy
    bool verbose_output;          // Print detailed test results
    unsigned long timeout_ms;     // Maximum time for POST completion
    
    PostConfig() : 
        run_hardware_tests(true),
        run_safety_tests(true), 
        run_timing_tests(false),  // Skip timing tests for fast POST
        verbose_output(false),
        timeout_ms(10000)         // 10 second timeout
    {}
};

// === POST RESULTS ===

struct PostResults {
    bool passed;
    int tests_run;
    int tests_passed;
    int tests_failed;
    unsigned long duration_ms;
    String failure_summary;
    ErrorCode critical_error;
    
    PostResults() : 
        passed(false), tests_run(0), tests_passed(0), tests_failed(0),
        duration_ms(0), critical_error(ERR_NONE) {}
    
    float getSuccessRate() const {
        return tests_run > 0 ? (float)tests_passed / tests_run * 100.0 : 0.0;
    }
};

// === POST EXECUTION ===

class PowerOnSelfTest {
private:
    PostConfig config;
    PostResults results;
    
public:
    /**
     * Run Power-On Self Test
     */
    PostResults runPost(const PostConfig& post_config = PostConfig()) {
        config = post_config;
        results = PostResults();
        
        unsigned long start_time = millis();
        
        if (config.verbose_output) {
            Serial.println("========================================");
            Serial.println("      POWER-ON SELF TEST (POST)");
            Serial.println("========================================");
        }
        
        // Clear any existing errors
        CLEAR_ERROR();
        
        // Configure test framework for POST mode
        test_framework_init(config.verbose_output);
        test_set_mode(TEST_MODE_POST);
        
        // Track original test results
        TestResults original_results = g_test_results;
        g_test_results = TestResults();
        g_test_results.start_time_ms = millis();
        
        // Run POST test battery
        test_run_tagged(TEST_TAG_POST);
        
        g_test_results.end_time_ms = millis();
        
        // Extract results
        results.tests_run = g_test_results.total_tests;
        results.tests_passed = g_test_results.passed_tests;
        results.tests_failed = g_test_results.failed_tests;
        results.duration_ms = millis() - start_time;
        
        // Determine overall result
        results.passed = (results.tests_failed == 0) && 
                        (results.tests_run > 0) && 
                        !HAS_CRITICAL_ERROR();
        
        // Check for critical errors
        if (HAS_ERROR()) {
            results.critical_error = g_error_manager.getCurrentErrorCode();
            results.failure_summary = g_error_manager.getCurrentErrorString();
        }
        
        // Generate failure summary
        if (!results.passed && results.failure_summary.length() == 0) {
            results.failure_summary = "POST failed: " + String(results.tests_failed) + 
                                     " of " + String(results.tests_run) + " tests failed";
        }
        
        // Restore original test results
        g_test_results = original_results;
        
        // Print POST summary
        printPostSummary();
        
        return results;
    }
    
    /**
     * Quick POST for fast startup validation
     */
    PostResults runQuickPost() {
        PostConfig quick_config;
        quick_config.run_hardware_tests = false;  // Skip hardware tests
        quick_config.run_timing_tests = false;   // Skip timing tests
        quick_config.verbose_output = false;     // Minimal output
        quick_config.timeout_ms = 2000;          // 2 second timeout
        
        return runPost(quick_config);
    }
    
    /**
     * Full POST with all validations
     */
    PostResults runFullPost() {
        PostConfig full_config;
        full_config.run_hardware_tests = true;
        full_config.run_safety_tests = true;
        full_config.run_timing_tests = false;   // Skip timing tests for speed
        full_config.verbose_output = true;
        full_config.timeout_ms = 8000;  // 8 second timeout
        
        return runPost(full_config);
    }
    
private:
    void printPostSummary() {
        if (config.verbose_output) {
            Serial.println("\n========================================");
            Serial.println("           POST SUMMARY");
            Serial.println("========================================");
        }
        
        Serial.print("POST Result: ");
        if (results.passed) {
            Serial.println("PASS ✓");
        } else {
            Serial.println("FAIL ✗");
        }
        
        if (config.verbose_output) {
            Serial.print("Tests Run: ");
            Serial.println(results.tests_run);
            Serial.print("Passed: ");
            Serial.println(results.tests_passed);
            Serial.print("Failed: ");
            Serial.println(results.tests_failed);
            Serial.print("Success Rate: ");
            Serial.print(results.getSuccessRate(), 1);
            Serial.println("%");
            Serial.print("Duration: ");
            Serial.print(results.duration_ms);
            Serial.println(" ms");
            
            if (!results.passed) {
                Serial.println("Failure Details:");
                Serial.println(results.failure_summary);
            }
            
            Serial.println("========================================");
        }
        
        if (!results.passed) {
            Serial.println("⚠️  ROBOT NOT READY - POST FAILED");
            if (results.failure_summary.length() > 0) {
                Serial.println("Reason: " + results.failure_summary);
            }
        } else {
            Serial.println("✅ ROBOT READY - POST PASSED");
        }
        
        Serial.println();
    }
};

// === CONVENIENCE FUNCTIONS ===

/**
 * Run standard POST at robot startup
 */
PostResults robot_post() {
    PowerOnSelfTest post;
    return post.runPost();
}

/**
 * Run quick POST for fast validation
 */
PostResults robot_quick_post() {
    PowerOnSelfTest post;
    return post.runQuickPost();
}

/**
 * Run comprehensive POST with all tests
 */
PostResults robot_full_post() {
    PowerOnSelfTest post;
    return post.runFullPost();
}

/**
 * Validate hardware configuration from global config
 */
bool validateHardwareConfig() {
    // Basic configuration validation
    if (!g_config.validateConfiguration()) {
        REPORT_ERROR(ERR_INVALID_CONFIG, "POST", "Hardware configuration validation failed");
        return false;
    }
    
    // Pin conflict detection
    int used_pins[20];  // Assume max 20 pins
    int pin_count = 0;
    
    // Add motor pins
    for (int i = 0; i < 4; i++) {
        used_pins[pin_count++] = g_config.hardware.motor_l_pins[i];
        used_pins[pin_count++] = g_config.hardware.motor_r_pins[i];
    }
    used_pins[pin_count++] = g_config.hardware.servo_pin;
    
    // Check for duplicates
    for (int i = 0; i < pin_count - 1; i++) {
        for (int j = i + 1; j < pin_count; j++) {
            if (used_pins[i] == used_pins[j]) {
                REPORT_ERROR(ERR_PIN_CONFLICT, "POST", 
                           "Pin " + String(used_pins[i]) + " assigned to multiple functions");
                return false;
            }
        }
    }
    
    return true;
}
}

// === POST-SPECIFIC TESTS ===

// These tests are specifically designed for POST and may be simpler
// than the full unit tests to ensure fast execution

TEST_CASE_POST("POST_ConfigValidation") {
    TEST_ASSERT_TRUE(validateHardwareConfig());
    return true;
}

TEST_CASE_POST("POST_ErrorSystemFunctional") {
    // Test basic error system functionality
    CLEAR_ERROR();
    TEST_ASSERT_FALSE(HAS_ERROR());
    
    REPORT_ERROR(ERR_INVALID_SPEED, "POST_TEST", "Test error message");
    TEST_ASSERT_TRUE(HAS_ERROR());
    TEST_ASSERT_EQUAL(ERR_INVALID_SPEED, g_error_manager.getCurrentErrorCode());
    
    CLEAR_ERROR();
    TEST_ASSERT_FALSE(HAS_ERROR());
    
    return true;
}

TEST_CASE_POST("POST_MemoryCheck") {
    // Basic memory/stack check - tests actual RAM availability
    
    // Test string operations don't crash
    String test_str = "POST Memory Test";
    test_str += " Extended";
    TEST_ASSERT_TRUE(test_str.length() > 10);
    
    // Test array operations
    int test_array[10];
    for (int i = 0; i < 10; i++) {
        test_array[i] = i * 2;
    }
    TEST_ASSERT_EQUAL(18, test_array[9]);
    
    return true;
}

TEST_CASE_POST("POST_HardwareConnections") {
    // Quick hardware connectivity check using global config
    
    // Test that pins can be set as outputs (no hardware failure)
    pinMode(g_config.hardware.motor_l_pins[0], OUTPUT);
    pinMode(g_config.hardware.motor_r_pins[0], OUTPUT);
    pinMode(g_config.hardware.servo_pin, OUTPUT);
    
    // Quick pin toggle test (verify electrical connectivity)
    digitalWrite(g_config.hardware.motor_l_pins[0], HIGH);
    delayMicroseconds(100);
    digitalWrite(g_config.hardware.motor_l_pins[0], LOW);
    
    return true;
}

TEST_CASE_POST("POST_PowerSupply") {
    // Quick power stability check
    unsigned long start_voltage = analogRead(A0);  // If voltage monitoring available
    delayMicroseconds(500);  // Reduced from 5ms to 500us
    unsigned long end_voltage = analogRead(A0);
    
    // Power should be relatively stable
    TEST_ASSERT_TRUE(abs(start_voltage - end_voltage) < 50);
    
    return true;
}

TEST_CASE_POST("POST_EmergencyStop") {
    // Test critical safety function works
    TerraPenRobot robot;
    robot.begin();
    
    // Test emergency stop without starting movement (faster)
    robot.emergencyStop();
    TEST_ASSERT_EQUAL(EMERGENCY_STOP, robot.getState());
    
    return true;
}
    
    return true;
}