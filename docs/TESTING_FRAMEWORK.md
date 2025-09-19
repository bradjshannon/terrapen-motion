# TerraPen Testing Framework Documentation

**Version**: 1.0  
**Date**: September 19, 2025  
**System**: Comprehensive Arduino unit testing with POST validation

## Overview

The TerraPen Motion Control project includes a complete testing framework designed for Arduino embedded systems. This framework provides unit testing, Power-On Self Test (POST), performance monitoring, and automated quality assurance.

## Testing Framework Architecture

### Core Components

#### 1. TestFramework (src/testing/TestFramework.h/cpp)
- **Purpose**: Custom Arduino unit testing framework
- **Features**: 
  - Lightweight assertion macros
  - Test tagging system (LOGIC, HARDWARE, POST, TIMING, STRESS)
  - Test discovery and execution engine
  - Results reporting and statistics
- **Usage**:
  ```cpp
  TEST_CASE_LOGIC("MyLogicTest") {
      TEST_ASSERT_TRUE(someCondition);
      TEST_ASSERT_EQUAL(expected, actual);
      return true;
  }
  ```

#### 2. Power-On Self Test (src/testing/PowerOnSelfTest.h)
- **Purpose**: Runtime validation of critical systems
- **Features**:
  - Quick POST (2 seconds) vs Full POST (30 seconds)
  - Hardware connectivity validation
  - Safety system verification
  - Essential functionality checks
- **Usage**:
  ```cpp
  PostResults results = runQuickPost();
  if (!results.passed) {
      handlePostFailure(results);
  }
  ```

#### 3. Performance Monitor (src/PerformanceMonitor.h/cpp)
- **Purpose**: Real-time system performance tracking
- **Features**:
  - CPU utilization monitoring
  - Memory usage tracking
  - Loop timing analysis
  - Step rate performance
  - NVRAM data logging
- **Usage**:
  ```cpp
  performance_monitor.begin();
  PerformanceMetrics metrics = performance_monitor.getMetrics();
  ```

## Test Categories and Tagging

### Test Tags

| Tag | Purpose | When to Run | Example Tests |
|-----|---------|-------------|---------------|
| `TEST_TAG_LOGIC` | Algorithm validation | Compile-time, CI/CD | Input validation, state machine logic |
| `TEST_TAG_HARDWARE` | Hardware interaction | Development, debugging | Pin connectivity, motor response |
| `TEST_TAG_POST` | Runtime validation | Every startup | Critical safety checks |
| `TEST_TAG_TIMING` | Performance validation | Development, optimization | Step timing accuracy |
| `TEST_TAG_STRESS` | Load testing | Periodic validation | Continuous operation limits |

### Usage Examples

```cpp
// Logic test - validates algorithms, runs fast
TEST_CASE_LOGIC("StepperDriver_SpeedCalculation") {
    StepperDriver motor;
    motor.setSpeed(100.0);
    TEST_ASSERT_EQUAL(1000, motor.getStepDelayMicros());
    return true;
}

// Hardware test - validates actual hardware interaction
TEST_CASE_HARDWARE("StepperDriver_ActualMovement") {
    StepperDriver motor;
    motor.begin(motor_pins);
    TEST_ASSERT_TRUE(motor.stepForward());
    return true;
}

// POST test - essential runtime validation
TEST_CASE_POST("POST_EmergencyStop") {
    TerraPenRobot robot;
    robot.begin(config);
    robot.moveForward(100);
    robot.emergencyStop();
    TEST_ASSERT_EQUAL(EMERGENCY_STOP, robot.getState());
    return true;
}
```

## Test Organization

### Directory Structure
```
test/
├── test_stepper_driver.cpp    # StepperDriver validation
├── test_servo_driver.cpp      # ServoDriver validation
└── test_terrapen_robot.cpp    # Robot integration tests

src/testing/
├── TestFramework.h/cpp        # Core testing framework
├── PowerOnSelfTest.h          # POST implementation
└── PerformanceMonitor.h/cpp   # Performance tracking

examples/TestRunner/
└── TestRunner.ino             # Interactive test execution
```

### Test File Structure
```cpp
// test_component.cpp
#include "../src/testing/TestFramework.h"
#include "../src/hardware/ComponentToTest.h"

// === INITIALIZATION TESTS ===
TEST_CASE_LOGIC("Component_DefaultInitialization") {
    // Test default state
}

// === FUNCTIONALITY TESTS ===
TEST_CASE_LOGIC("Component_BasicOperation") {
    // Test core functionality
}

// === HARDWARE TESTS ===
TEST_CASE_HARDWARE("Component_HardwareInteraction") {
    // Test with actual hardware
}

// === ERROR CONDITION TESTS ===
TEST_CASE_LOGIC("Component_ErrorHandling") {
    // Test error conditions
}

// === POST TESTS ===
TEST_CASE_POST("Component_CriticalValidation") {
    // Essential runtime checks
}
```

## Test Execution

### Manual Execution
1. **Upload TestRunner Example**:
   ```
   File → Examples → TerraPenMotionControl → TestRunner
   ```

2. **Interactive Menu**:
   ```
   === TerraPen Test Runner ===
   1. Run All Unit Tests
   2. Run POST Tests Only
   3. Performance Monitoring
   4. Run Specific Test
   5. System Configuration
   6. Storage Status
   ```

### Automated Execution

#### Pre-commit Hooks
```bash
# Install git hooks
git config core.hooksPath .githooks

# Pre-commit will run:
# 1. Compilation verification
# 2. Unit test compilation check  
# 3. Documentation validation
# 4. Python simulator tests (if available)
```

#### Continuous Integration
```yaml
# Example CI workflow
name: TerraPen Testing
on: [push, pull_request]
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install Arduino CLI
        run: |
          curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
          arduino-cli core update-index
          arduino-cli core install arduino:avr
      - name: Compile Tests
        run: |
          arduino-cli compile --fqbn arduino:avr:nano:cpu=atmega328old \
                             --library . examples/TestRunner
```

## Performance Monitoring

### Real-time Metrics
- **CPU Usage**: Percentage of time spent in active processing
- **Memory Usage**: Free RAM and stack usage
- **Loop Timing**: Average and peak loop execution time
- **Step Rate**: Actual steps per second vs target
- **Error Rate**: Errors per minute and error types

### Data Storage
- **NVRAM Logging**: Circular buffer with 30+ performance records
- **ESP32 Upload**: Automatic data transmission when connected
- **Data Retention**: 72-hour default retention with configurable cleanup

### Performance Thresholds
```cpp
// Configurable performance alerts
PerformanceConfig performance = {
    .cpu_warning_threshold = 80,      // 80% CPU usage warning
    .cpu_critical_threshold = 95,     // 95% CPU usage critical
    .memory_warning_bytes = 200,      // Low memory warning
    .memory_critical_bytes = 50,      // Critical memory level
    .timing_warning_us = 50000,       // 50ms loop time warning
    .timing_critical_us = 100000      // 100ms loop time critical
};
```

## Error Handling Integration

### Error System Features
- **60+ Specific Error Codes**: Detailed error classification
- **Context Tracking**: File, line, function where error occurred
- **Error History**: Recent error log with timestamps
- **Automatic Recovery**: Configurable recovery attempts
- **Performance Impact**: Error rate tracking in performance metrics

### Error Code Categories
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

## Best Practices

### Writing Tests
1. **Test One Thing**: Each test should validate a single behavior
2. **Use Descriptive Names**: Test names should clearly indicate what's being tested
3. **Appropriate Tags**: Use correct tags for proper test categorization
4. **Setup/Teardown**: Clean state between tests
5. **Error Conditions**: Test both success and failure paths

### Performance Considerations
- **Lightweight Assertions**: Minimal overhead for production use
- **Selective Execution**: Use tags to run only necessary tests
- **Resource Management**: Clean up allocated resources
- **Timing Sensitivity**: Account for Arduino timing limitations

### Maintenance
- **Regular Review**: Update tests as code evolves
- **Performance Baselines**: Establish and monitor performance benchmarks
- **Documentation**: Keep test documentation current
- **Integration**: Ensure tests work with CI/CD pipeline

## Integration with Development Workflow

### Development Cycle
1. **Write Test**: Create test for new feature
2. **Implement**: Write minimal code to pass test
3. **Validate**: Run full test suite
4. **Performance Check**: Monitor resource usage
5. **Commit**: Automated pre-commit testing
6. **Deploy**: POST validation on target hardware

### Quality Gates
- **Pre-commit**: Compilation and unit test validation
- **Pre-merge**: Full test suite and performance regression check
- **Pre-deploy**: Hardware-in-loop validation
- **Runtime**: Continuous POST and performance monitoring

This testing framework provides comprehensive quality assurance while maintaining the performance and resource constraints of embedded Arduino systems.