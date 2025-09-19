# TerraPen Motion Control Library - Testing Guide

**Version**: 2.0.0  
**Date**: September 19, 2025  
**Status**: ✅ Complete testing framework with multiple execution methods

## Quick Start - How to Run Tests

### ✅ Method 1: TestRunner Example (Recommended)

**Best for most users** - Complete test suite with interactive interface:

1. **Open TestRunner:**
   ```
   File → Examples → TerraPenMotionControl → TestRunner
   ```

2. **Upload to Arduino Nano:**
   - Connect Arduino via USB
   - Select **Tools → Board → Arduino Nano**
   - Select **Tools → Port → [Your COM Port]**
   - Click **Upload**

3. **View Results:**
   - Open **Tools → Serial Monitor** (115200 baud)
   - Watch comprehensive test execution

**Expected Output:**
```
=== TerraPen Test Runner ===
✅ POST: Hardware initialization
✅ POST: Configuration validation  
✅ Unit Tests: 25/25 PASSED
✅ Hardware Tests: 12/12 PASSED
✅ Performance: CPU 45%, RAM 50%
=== ALL TESTS PASSED ===
```

### ✅ Method 2: PlatformIO Testing (Advanced)

**For command-line users:**
```bash
# Logic tests (no hardware required)
pio test -e test

# Full integration tests (hardware required)
pio test -e nano
```

### ✅ Method 3: Individual Component Tests

Run specific test categories:
- `examples/StepperTest/` - Stepper motor validation
- `examples/ServoTest/` - Servo control validation
- `examples/HardwareTest/` - Combined hardware tests
- `examples/SimpleCoordinateTest/` - Basic coordinate movement

## Test Framework Architecture

### Test Categories

| Category | Purpose | Hardware Required | Duration |
|----------|---------|-------------------|-----------|
| **Logic Tests** | Algorithm validation | No | 10-30 sec |
| **Hardware Tests** | Driver functionality | Yes | 1-3 min |
| **POST Tests** | Runtime validation | Yes | 2-10 sec |
| **Integration Tests** | Full system | Yes | 3-5 min |

### Test Macros

```cpp
// Logic test - pure algorithms, no hardware
TEST_CASE_LOGIC("Position_Calculations") {
    Position p1(0, 0, 0);
    Position p2 = p1.moveForward(10);
    TEST_ASSERT_EQUAL_FLOAT(10.0, p2.y);
    return true;
}

// Hardware test - actual hardware interaction
TEST_CASE_HARDWARE("StepperDriver_Movement") {
    StepperDriver motor;
    motor.begin(2, 3, 4, 5);
    TEST_ASSERT_TRUE(motor.stepForward());
    return true;
}

// POST test - essential runtime validation
TEST_CASE_POST("POST_EmergencyStop") {
    TerraPenRobot robot;
    robot.begin(config);
    robot.emergencyStop();
    TEST_ASSERT_EQUAL(EMERGENCY_STOP, robot.getState());
    return true;
}
```

## What Tests Validate

### ✅ Library Code Correctness
**Unit tests confirm the library's code is valid and correct:**

1. **Algorithm Validation** - Position calculations, kinematics, coordinate transformations
2. **API Behavior** - Method calls return expected results and handle edge cases
3. **Hardware Integration** - Drivers correctly control stepper motors and servos  
4. **System Coordination** - Robot state management, movement coordination, safety features
5. **Mathematical Accuracy** - Trigonometry, distance calculations, angle conversions

### ✅ Test Coverage (25+ Tests)
- **Robot Control**: Initialization, state management, coordinate movement
- **Hardware Drivers**: Stepper timing, servo control, pin configuration
- **Position System**: Coordinate calculations, workspace boundaries
- **Performance**: Memory usage, timing accuracy, loop frequency
- **Safety Features**: Emergency stop, error recovery, boundary checking

## Development Workflow

### 🔄 Recommended Testing Cycle

1. **Before Code Changes:**
   ```cpp
   // Run POST to verify baseline
   PostResults results = runQuickPost();
   ```

2. **After Implementation:**
   ```cpp
   // Run relevant unit tests
   runTests("LOGIC");     // Quick validation
   runTests("HARDWARE");  // Hardware validation if needed
   ```

3. **Before Committing:**
   ```cpp
   // Run complete test suite
   runTests("ALL");
   ```

### 📊 Performance Monitoring

**Real-time metrics during testing:**
```cpp
PerformanceMonitor monitor;
monitor.begin();

// During operation
PerformanceMetrics metrics = monitor.getMetrics();
Serial.printf("CPU: %.1f%%, RAM: %d bytes, Freq: %.1f Hz\n", 
              metrics.cpu_utilization_percent,
              metrics.free_memory_bytes,
              metrics.update_frequency_hz);
```

**Configurable thresholds:**
- CPU usage warning: 80%, critical: 95%
- Memory warning: 200 bytes, critical: 50 bytes
- Loop timing warning: 50ms, critical: 100ms

## Power-On Self Test (POST)

**Automatic validation at startup:**
```cpp
void setup() {
    Serial.begin(115200);
    
    // Essential runtime validation
    PostResults result = runQuickPost();
    if (!result.passed) {
        Serial.println("❌ POST Failed: " + result.failure_summary);
        while(1) delay(1000);  // Stop execution
    }
    
    Serial.println("✅ POST Passed - System Ready");
    // Continue with normal setup...
}
```

**POST Categories:**
- **Hardware connectivity** - Stepper/servo pin verification
- **Memory validation** - Available RAM and EEPROM
- **Configuration checks** - Valid settings and calibration
- **Safety systems** - Emergency stop functionality

## Continuous Integration

### GitHub Actions Setup
```yaml
# .github/workflows/test.yml
name: TerraPen Tests
on: [push, pull_request]
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Setup Arduino CLI
        run: arduino-cli core install arduino:avr
      - name: Compile Tests
        run: arduino-cli compile --fqbn arduino:avr:nano examples/TestRunner
```

### Git Pre-commit Hook
```bash
# .git/hooks/pre-commit
echo "Running TerraPen tests..."
pio test -e test
if [ $? -ne 0 ]; then
    echo "❌ Tests failed - commit aborted"
    exit 1
fi
```

## Troubleshooting

### Common Issues

**❌ Tests hang or timeout:**
- Check hardware connections and power supply
- Increase timeout values in test configuration
- Run individual tests to isolate problems

**❌ Memory-related crashes:**
- Monitor free memory during tests
- Reduce test batch sizes
- Check for memory leaks in test code

**❌ Random hardware test failures:**
- Verify stable power supply (stepper motors need adequate current)
- Add delays between hardware operations
- Check wiring and connections

**❌ Performance issues:**
- Profile individual functions for bottlenecks
- Optimize timing-critical code paths
- Consider reducing update frequency

### Test Result Interpretation

**✅ Successful Output:**
```
=== TEST RESULTS ===
Logic Tests:     25/25 PASSED
Hardware Tests:  12/12 PASSED  
Integration:     8/8 PASSED
POST:           PASSED
Performance:    GOOD (CPU: 45%, RAM: 50%)
=== ALL TESTS PASSED ===
```

**❌ Failed Output:**
```
=== TEST RESULTS ===
❌ FAILED: Position_RotationCalculation
   Expected: 1.57, Actual: 1.56, Delta: 0.01
❌ FAILED: StepperDriver_TimingAccuracy  
   Timing deviation: 15% (max 10%)
=== 2 TESTS FAILED ===
```

## Best Practices

### Writing New Tests
1. **Test one behavior** - Each test validates a single function or feature
2. **Use descriptive names** - Clearly indicate what's being tested
3. **Appropriate categories** - Use correct TEST_CASE_* macros
4. **Clean state** - Reset hardware/software state between tests
5. **Test edge cases** - Include boundary conditions and error paths

### Maintenance
- **Regular execution** - Run tests with every code change
- **Update with features** - Add tests for new functionality
- **Performance baselines** - Monitor and update performance expectations
- **Documentation sync** - Keep test documentation current

## Summary

**✅ Complete Testing Framework Available**

The TerraPen Motion Control Library provides comprehensive testing without requiring main.cpp or complex setup:

- **Primary Method**: Use `examples/TestRunner/TestRunner.ino` for complete validation
- **25+ Tests**: Logic, hardware, integration, and performance tests
- **Multiple Interfaces**: Arduino IDE, PlatformIO, command line
- **Real-time Monitoring**: Performance metrics and error tracking
- **CI/CD Ready**: Automated testing support for development workflows

**The library's testing framework validates code correctness and ensures reliable robot operation.**