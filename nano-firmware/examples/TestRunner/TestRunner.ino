/**
 * Test Runner Example
 * 
 * Arduino sketch that demonstrates how to use the TerraPen testing framework.
 * This example shows different ways to run tests and can be used as a template.
 *
 * Features:
 * - Complete test suite execution
 * - Interactive serial command interface  
 * - Multiple test execution modes
 * - System configuration display
 * - Real-time status monitoring
 */

#include <TerraPenMotionControl.h>

// Include testing framework
#include "test/framework/TestFramework.h"
#include "test/framework/PowerOnSelfTest.h"
#include "src/ErrorSystem.h"
#include "src/PerformanceMonitor.h"

// Include all test files
#include "test/hardware/test_stepper_driver.cpp"
#include "test/hardware/test_servo_driver.cpp"
#include "test/unit/test_terrapen_robot.cpp"
#include "test/integration/test_phase2_coordinate_system.cpp"

// Test configuration
bool run_post_on_startup = true;
bool continuous_performance_monitoring = false;
bool verbose_output = true;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {
    delay(10);  // Wait for serial connection
  }
  
  Serial.println("=====================================");
  Serial.println("     TerraPen Test Runner v1.0");
  Serial.println("=====================================");
  Serial.println();
  
  // Initialize systems
  test_framework_init(verbose_output);
  g_error_manager.setErrorReporting(true);
  g_performance_monitor.setDetailedLogging(false);
  
  // Run POST if enabled
  if (run_post_on_startup) {
    Serial.println("Running Power-On Self Test...");
    PostResults post_result = robot_post();
    
    if (!post_result.passed) {
      Serial.println("⚠️  POST FAILED - Robot may not be operational");
      Serial.println("Fix hardware issues before proceeding");
      Serial.println();
    } else {
      Serial.println("✅ POST PASSED - Robot ready for operation");
      Serial.println();
    }
  }
  
  // Print menu
  printMenu();
}

void loop() {
  // Handle serial commands
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    
    handleCommand(command);
  }
  
  // Continuous performance monitoring
  if (continuous_performance_monitoring) {
    static unsigned long last_perf_report = 0;
    if (millis() - last_perf_report > 5000) {
      g_performance_monitor.printSummary();
      last_perf_report = millis();
    }
  }
  
  delay(100);
}

void printMenu() {
  Serial.println("=== Test Runner Commands ===");
  Serial.println("Basic Tests:");
  Serial.println("  post      - Run Power-On Self Test");
  Serial.println("  unit      - Run all unit tests");
  Serial.println("  hardware  - Run hardware tests only");
  Serial.println("  logic     - Run logic tests only");
  Serial.println("  all       - Run all tests");
  Serial.println();
  Serial.println("Performance:");
  Serial.println("  perf      - Show performance summary");
  Serial.println("  perfon    - Enable continuous performance monitoring");
  Serial.println("  perfoff   - Disable continuous performance monitoring");
  Serial.println("  perfreset - Reset performance counters");
  Serial.println();
  Serial.println("Error System:");
  Serial.println("  errors    - Show current errors");
  Serial.println("  history   - Show error history");
  Serial.println("  clearerr  - Clear current errors");
  Serial.println();
  Serial.println("Configuration:");
  Serial.println("  verbose   - Toggle verbose output");
  Serial.println("  menu      - Show this menu");
  Serial.println("  status    - Show system status");
  Serial.println();
  Serial.print("Enter command: ");
}

void handleCommand(const String& command) {
  Serial.println(command);  // Echo command
  Serial.println();
  
  if (command == "post") {
    runPostTest();
  } else if (command == "unit") {
    runUnitTests();
  } else if (command == "hardware") {
    runHardwareTests();
  } else if (command == "logic") {
    runLogicTests();
  } else if (command == "all") {
    runAllTests();
  } else if (command == "perf") {
    showPerformanceSummary();
  } else if (command == "perfon") {
    enablePerformanceMonitoring();
  } else if (command == "perfoff") {
    disablePerformanceMonitoring();
  } else if (command == "perfreset") {
    resetPerformanceCounters();
  } else if (command == "errors") {
    showCurrentErrors();
  } else if (command == "history") {
    showErrorHistory();
  } else if (command == "clearerr") {
    clearErrors();
  } else if (command == "verbose") {
    toggleVerboseOutput();
  } else if (command == "menu") {
    printMenu();
  } else if (command == "status") {
    showSystemStatus();
  } else {
    Serial.println("Unknown command. Type 'menu' for help.");
  }
  
  Serial.println();
  Serial.print("Enter command: ");
}

// === TEST EXECUTION FUNCTIONS ===

void runPostTest() {
  Serial.println("Running Power-On Self Test...");
  
  PowerOnSelfTest post;
  PostResults result = post.runPost();
  
  if (result.passed) {
    Serial.println("✅ POST completed successfully");
  } else {
    Serial.println("❌ POST failed");
    Serial.println("Details: " + result.failure_summary);
  }
}

void runUnitTests() {
  Serial.println("Running all unit tests...");
  
  test_framework_init(verbose_output);
  test_set_mode(TEST_MODE_ALL);
  test_run_all();
  
  reportTestResults();
}

void runHardwareTests() {
  Serial.println("Running hardware tests only...");
  Serial.println("⚠️  Make sure robot hardware is connected and powered");
  
  test_framework_init(verbose_output);
  test_set_mode(TEST_MODE_HARDWARE);
  test_run_all();
  
  reportTestResults();
}

void runLogicTests() {
  Serial.println("Running logic tests only...");
  
  test_framework_init(verbose_output);
  test_set_mode(TEST_MODE_LOGIC);
  test_run_all();
  
  reportTestResults();
}

void runAllTests() {
  Serial.println("Running comprehensive test suite...");
  Serial.println("This may take several minutes...");
  
  // 1. POST first
  Serial.println("\n1. Power-On Self Test");
  PostResults post_result = robot_post();
  
  if (!post_result.passed) {
    Serial.println("❌ POST failed - skipping other tests");
    return;
  }
  
  // 2. Logic tests (fast)
  Serial.println("\n2. Logic Tests");
  test_framework_init(false);  // Non-verbose for speed
  test_set_mode(TEST_MODE_LOGIC);
  test_run_all();
  
  // 3. Hardware tests (slow)
  Serial.println("\n3. Hardware Tests");
  test_set_mode(TEST_MODE_HARDWARE);
  test_run_all();
  
  // 4. Overall results
  Serial.println("\n=== COMPREHENSIVE TEST RESULTS ===");
  Serial.print("POST: ");
  Serial.println(post_result.passed ? "PASS" : "FAIL");
  
  reportTestResults();
  
  if (test_all_passed() && post_result.passed) {
    Serial.println("🎉 ALL TESTS PASSED - ROBOT FULLY VALIDATED");
  } else {
    Serial.println("❌ SOME TESTS FAILED - CHECK ROBOT BEFORE USE");
  }
}

void reportTestResults() {
  if (test_all_passed()) {
    Serial.println("✅ All tests passed");
  } else {
    Serial.println("❌ Some tests failed");
  }
  
  // Print test results JSON for automated processing
  if (verbose_output) {
    Serial.println("Results JSON: " + test_results_json());
  }
}

// === PERFORMANCE MONITORING ===

void showPerformanceSummary() {
  Serial.println("=== Performance Summary ===");
  g_performance_monitor.printSummary();
}

void enablePerformanceMonitoring() {
  continuous_performance_monitoring = true;
  g_performance_monitor.setDetailedLogging(true);
  Serial.println("Continuous performance monitoring enabled");
  Serial.println("Performance reports will appear every 5 seconds");
}

void disablePerformanceMonitoring() {
  continuous_performance_monitoring = false;
  g_performance_monitor.setDetailedLogging(false);
  Serial.println("Continuous performance monitoring disabled");
}

void resetPerformanceCounters() {
  g_performance_monitor.resetMetrics();
  Serial.println("Performance counters reset");
}

// === ERROR MANAGEMENT ===

void showCurrentErrors() {
  if (HAS_ERROR()) {
    Serial.println("Current Error:");
    Serial.println(GET_ERROR_STRING());
    
    if (HAS_CRITICAL_ERROR()) {
      Serial.println("⚠️  CRITICAL ERROR - System should be stopped");
    }
  } else {
    Serial.println("No current errors");
  }
}

void showErrorHistory() {
  Serial.println("=== Error History ===");
  g_error_manager.printErrorHistory();
}

void clearErrors() {
  CLEAR_ERROR();
  Serial.println("All errors cleared");
}

// === CONFIGURATION ===

void toggleVerboseOutput() {
  verbose_output = !verbose_output;
  Serial.print("Verbose output: ");
  Serial.println(verbose_output ? "ON" : "OFF");
}

void showSystemStatus() {
  Serial.println("=== System Status ===");
  
  // Error status
  Serial.print("Errors: ");
  if (HAS_ERROR()) {
    Serial.println("YES");
    Serial.println("  " + GET_ERROR_STRING());
  } else {
    Serial.println("NONE");
  }
  
  // Performance status
  PerformanceMetrics perf = g_performance_monitor.getMetrics();
  Serial.print("Performance Monitoring: ");
  Serial.println(continuous_performance_monitoring ? "ENABLED" : "DISABLED");
  
  if (perf.total_updates > 0) {
    Serial.print("Update Frequency: ");
    Serial.print(perf.update_frequency_hz, 1);
    Serial.println(" Hz");
    
    Serial.print("CPU Utilization: ");
    Serial.print(perf.cpu_utilization_percent, 1);
    Serial.println("%");
  }
  
  // Memory status
  if (perf.free_memory_bytes >= 0) {
    Serial.print("Free Memory: ");
    Serial.print(perf.free_memory_bytes);
    Serial.println(" bytes");
  }
  
  // Configuration
  Serial.print("Verbose Output: ");
  Serial.println(verbose_output ? "ON" : "OFF");
  
  Serial.print("Test Framework: ");
  Serial.println("READY");
  
  Serial.println("===================");
}