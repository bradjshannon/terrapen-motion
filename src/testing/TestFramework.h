#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <Arduino.h>

/**
 * Arduino Unit Testing Framework
 * 
 * Lightweight testing framework designed for embedded systems with limited resources.
 * Provides assertion macros, test organization, and result reporting.
 * 
 * Usage:
 *   TEST_SUITE("Motor Tests") {
 *     TEST_CASE("Stepper Timing") {
 *       StepperDriver motor;
 *       motor.begin(2, 3, 4, 5);
 *       TEST_ASSERT_TRUE(motor.isInitialized());
 *     }
 *   }
 *   
 *   void setup() {
 *     TEST_RUN_ALL();
 *   }
 */

// Test result tracking
struct TestResults {
    int total_tests;
    int passed_tests;
    int failed_tests;
    int skipped_tests;
    unsigned long start_time_ms;
    unsigned long end_time_ms;
    
    TestResults() : total_tests(0), passed_tests(0), failed_tests(0), skipped_tests(0), 
                   start_time_ms(0), end_time_ms(0) {}
    
    float getSuccessRate() const {
        return total_tests > 0 ? (float)passed_tests / total_tests * 100.0 : 0.0;
    }
    
    unsigned long getDurationMs() const {
        return end_time_ms - start_time_ms;
    }
};

// Global test results
extern TestResults g_test_results;
extern bool g_test_verbose;
extern String g_current_suite;
extern String g_current_test;

// Test configuration
enum TestMode {
    TEST_MODE_ALL,        // Run all tests
    TEST_MODE_HARDWARE,   // Hardware validation tests only
    TEST_MODE_LOGIC,      // Logic/algorithm tests only
    TEST_MODE_POST        // Power-On Self Test subset
};

extern TestMode g_test_mode;

// === ASSERTION MACROS ===

#define TEST_ASSERT_TRUE(condition) \
    do { \
        g_test_results.total_tests++; \
        if (!(condition)) { \
            g_test_results.failed_tests++; \
            if (g_test_verbose) { \
                Serial.print("FAIL: "); \
                Serial.print(g_current_suite); \
                Serial.print(" > "); \
                Serial.print(g_current_test); \
                Serial.print(" - "); \
                Serial.print(#condition); \
                Serial.print(" at line "); \
                Serial.println(__LINE__); \
            } \
            return false; \
        } else { \
            g_test_results.passed_tests++; \
            if (g_test_verbose) { \
                Serial.print("PASS: "); \
                Serial.print(g_current_test); \
                Serial.print(" - "); \
                Serial.println(#condition); \
            } \
        } \
    } while(0)

#define TEST_ASSERT_FALSE(condition) TEST_ASSERT_TRUE(!(condition))

#define TEST_ASSERT_EQUAL(expected, actual) \
    do { \
        g_test_results.total_tests++; \
        if ((expected) != (actual)) { \
            g_test_results.failed_tests++; \
            if (g_test_verbose) { \
                Serial.print("FAIL: "); \
                Serial.print(g_current_suite); \
                Serial.print(" > "); \
                Serial.print(g_current_test); \
                Serial.print(" - Expected: "); \
                Serial.print(expected); \
                Serial.print(", Actual: "); \
                Serial.print(actual); \
                Serial.print(" at line "); \
                Serial.println(__LINE__); \
            } \
            return false; \
        } else { \
            g_test_results.passed_tests++; \
            if (g_test_verbose) { \
                Serial.print("PASS: "); \
                Serial.print(g_current_test); \
                Serial.print(" - "); \
                Serial.print(#expected); \
                Serial.print(" == "); \
                Serial.println(#actual); \
            } \
        } \
    } while(0)

#define TEST_ASSERT_NEAR(expected, actual, tolerance) \
    TEST_ASSERT_TRUE(abs((expected) - (actual)) <= (tolerance))

#define TEST_ASSERT_NULL(ptr) TEST_ASSERT_TRUE((ptr) == nullptr)
#define TEST_ASSERT_NOT_NULL(ptr) TEST_ASSERT_TRUE((ptr) != nullptr)

// Timing assertions
#define TEST_ASSERT_TIMING_BETWEEN(code, min_us, max_us) \
    do { \
        unsigned long start_time = micros(); \
        code; \
        unsigned long duration = micros() - start_time; \
        TEST_ASSERT_TRUE(duration >= (min_us) && duration <= (max_us)); \
    } while(0)

// === TEST ORGANIZATION MACROS ===

// Test battery tags for different test modes
#define TEST_TAG_HARDWARE   0x01
#define TEST_TAG_LOGIC      0x02
#define TEST_TAG_POST       0x04
#define TEST_TAG_ALL        0xFF

// Function prototype for test cases
typedef bool (*TestFunction)();

// Test case registration
struct TestCase {
    const char* name;
    TestFunction function;
    uint8_t tags;
    TestCase* next;
};

extern TestCase* g_test_list;

// Test registration macro
#define TEST_CASE_TAGGED(name, tags) \
    bool test_##name(); \
    TestCase test_case_##name = {#name, test_##name, tags, nullptr}; \
    __attribute__((constructor)) void register_##name() { \
        test_case_##name.next = g_test_list; \
        g_test_list = &test_case_##name; \
    } \
    bool test_##name()

// Convenience macros for common test types
#define TEST_CASE(name) TEST_CASE_TAGGED(name, TEST_TAG_ALL)
#define TEST_CASE_HARDWARE(name) TEST_CASE_TAGGED(name, TEST_TAG_HARDWARE | TEST_TAG_POST)
#define TEST_CASE_LOGIC(name) TEST_CASE_TAGGED(name, TEST_TAG_LOGIC)
#define TEST_CASE_POST(name) TEST_CASE_TAGGED(name, TEST_TAG_POST)

// Test suite organization (for reporting only)
#define TEST_SUITE(name) \
    void test_suite_##name() { \
        g_current_suite = name; \
        if (g_test_verbose) { \
            Serial.print("\n=== "); \
            Serial.print(name); \
            Serial.println(" ==="); \
        }

#define END_TEST_SUITE() }

// === TEST EXECUTION ===

void test_framework_init(bool verbose = true);
void test_set_mode(TestMode mode);
bool test_should_run(uint8_t test_tags);
void test_run_all();
void test_run_tagged(uint8_t tags);
void test_print_summary();

// Convenience functions
#define TEST_RUN_ALL() test_run_all()
#define TEST_RUN_POST() test_run_tagged(TEST_TAG_POST)
#define TEST_RUN_HARDWARE() test_run_tagged(TEST_TAG_HARDWARE)
#define TEST_RUN_LOGIC() test_run_tagged(TEST_TAG_LOGIC)

// === MOCK/STUB SUPPORT ===

// Simple mock framework for hardware dependencies
class MockDigitalPin {
private:
    int pin_number;
    int current_state;
    bool is_output;
    
public:
    MockDigitalPin(int pin) : pin_number(pin), current_state(LOW), is_output(false) {}
    
    void pinMode(int mode) { is_output = (mode == OUTPUT); }
    void digitalWrite(int value) { if (is_output) current_state = value; }
    int digitalRead() { return current_state; }
    
    // Test helpers
    void setMockState(int state) { current_state = state; }
    int getMockState() { return current_state; }
    bool isConfiguredAsOutput() { return is_output; }
};

// Time mocking for deterministic timing tests
class MockTimer {
private:
    static unsigned long mock_micros_value;
    static bool use_mock_time;
    
public:
    static void setMockMicros(unsigned long value) { 
        mock_micros_value = value; 
        use_mock_time = true; 
    }
    static void useMockTime(bool use_mock) { use_mock_time = use_mock; }
    static unsigned long getMicros() { 
        return use_mock_time ? mock_micros_value : micros(); 
    }
    static void advanceTime(unsigned long delta_us) { 
        if (use_mock_time) mock_micros_value += delta_us; 
    }
};

#endif // TEST_FRAMEWORK_H