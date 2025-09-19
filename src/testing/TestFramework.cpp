#include "TestFramework.h"

// Global test state
TestResults g_test_results;
bool g_test_verbose = true;
String g_current_suite = "";
String g_current_test = "";
TestMode g_test_mode = TEST_MODE_ALL;
TestCase* g_test_list = nullptr;

// MockTimer static members
unsigned long MockTimer::mock_micros_value = 0;
bool MockTimer::use_mock_time = false;

/**
 * Initialize the test framework
 */
void test_framework_init(bool verbose) {
    g_test_verbose = verbose;
    g_test_results = TestResults();
    
    if (verbose) {
        Serial.println("=================================");
        Serial.println("  TerraPen Unit Testing Framework");
        Serial.println("=================================");
    }
}

/**
 * Set test execution mode
 */
void test_set_mode(TestMode mode) {
    g_test_mode = mode;
    
    if (g_test_verbose) {
        Serial.print("Test mode: ");
        switch (mode) {
            case TEST_MODE_ALL: Serial.println("ALL TESTS"); break;
            case TEST_MODE_HARDWARE: Serial.println("HARDWARE ONLY"); break;
            case TEST_MODE_LOGIC: Serial.println("LOGIC ONLY"); break;
            case TEST_MODE_POST: Serial.println("POST (Power-On Self Test)"); break;
        }
    }
}

/**
 * Check if a test should run based on current mode and test tags
 */
bool test_should_run(uint8_t test_tags) {
    switch (g_test_mode) {
        case TEST_MODE_ALL:
            return true;
        case TEST_MODE_HARDWARE:
            return (test_tags & TEST_TAG_HARDWARE) != 0;
        case TEST_MODE_LOGIC:
            return (test_tags & TEST_TAG_LOGIC) != 0;
        case TEST_MODE_POST:
            return (test_tags & TEST_TAG_POST) != 0;
        default:
            return false;
    }
}

/**
 * Run all registered tests
 */
void test_run_all() {
    test_run_tagged(TEST_TAG_ALL);
}

/**
 * Run tests with specific tags
 */
void test_run_tagged(uint8_t tags) {
    if (g_test_verbose) {
        Serial.println("\nStarting test execution...");
    }
    
    g_test_results.start_time_ms = millis();
    
    // Count tests to run
    int tests_to_run = 0;
    TestCase* current = g_test_list;
    while (current != nullptr) {
        if ((current->tags & tags) != 0) {
            tests_to_run++;
        }
        current = current->next;
    }
    
    if (g_test_verbose) {
        Serial.print("Found ");
        Serial.print(tests_to_run);
        Serial.println(" tests to execute\n");
    }
    
    // Execute tests
    current = g_test_list;
    while (current != nullptr) {
        if ((current->tags & tags) != 0) {
            g_current_test = current->name;
            
            if (g_test_verbose) {
                Serial.print("Running: ");
                Serial.print(current->name);
                Serial.print("...");
            }
            
            // Execute test
            bool result = current->function();
            
            if (g_test_verbose) {
                Serial.println(result ? " PASS" : " FAIL");
            }
        }
        current = current->next;
    }
    
    g_test_results.end_time_ms = millis();
    
    // Print summary
    test_print_summary();
}

/**
 * Print test execution summary
 */
void test_print_summary() {
    Serial.println("\n=================================");
    Serial.println("         TEST SUMMARY");
    Serial.println("=================================");
    
    Serial.print("Total Tests:    ");
    Serial.println(g_test_results.total_tests);
    Serial.print("Passed:         ");
    Serial.println(g_test_results.passed_tests);
    Serial.print("Failed:         ");
    Serial.println(g_test_results.failed_tests);
    Serial.print("Success Rate:   ");
    Serial.print(g_test_results.getSuccessRate(), 1);
    Serial.println("%");
    
    Serial.print("Execution Time: ");
    Serial.print(g_test_results.getDurationMs());
    Serial.println(" ms");
    
    if (g_test_results.failed_tests == 0) {
        Serial.println("\n✓ ALL TESTS PASSED!");
    } else {
        Serial.println("\n✗ SOME TESTS FAILED!");
        Serial.print("Failed tests: ");
        Serial.println(g_test_results.failed_tests);
    }
    
    Serial.println("=================================\n");
}

/**
 * Quick test status check (for automated systems)
 */
bool test_all_passed() {
    return g_test_results.failed_tests == 0 && g_test_results.total_tests > 0;
}

/**
 * Get test results as JSON string (for CI integration)
 */
String test_results_json() {
    String json = "{";
    json += "\"total\":" + String(g_test_results.total_tests) + ",";
    json += "\"passed\":" + String(g_test_results.passed_tests) + ",";
    json += "\"failed\":" + String(g_test_results.failed_tests) + ",";
    json += "\"success_rate\":" + String(g_test_results.getSuccessRate()) + ",";
    json += "\"duration_ms\":" + String(g_test_results.getDurationMs());
    json += "}";
    return json;
}