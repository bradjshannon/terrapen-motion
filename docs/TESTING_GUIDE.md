# Testing Workflow and CI Integration Guide

This document explains how to integrate the TerraPen testing framework into your development workflow and set up automated testing.

## Overview

The TerraPen project includes a comprehensive testing system with four main components:

1. **Unit Testing Framework** - Validates individual components
2. **Error Handling System** - Provides detailed error reporting  
3. **POST (Power-On Self Test)** - Runtime validation at startup
4. **Performance Monitoring** - Real-time performance metrics

## Testing Best Practices

### When to Run Tests

**✅ CORRECT Testing Workflow:**

1. **Before any code changes** - Run POST to verify baseline functionality
2. **After implementing changes** - Run relevant unit tests
3. **Before committing code** - Run full test suite
4. **At startup** - Run POST to validate hardware

**❌ AVOID:**
- Committing without running tests
- Skipping tests for "small" changes
- Running only subset of tests for complex changes

### Test Types and When to Use Each

| Test Type | When to Use | Duration | Purpose |
|-----------|-------------|----------|---------|
| **POST** | Every startup | 2-10 sec | Hardware validation |
| **Unit Tests** | After code changes | 30-60 sec | Regression prevention |
| **Hardware Tests** | Before deployment | 2-5 min | Real hardware validation |
| **Integration Tests** | Major changes | 5-10 min | End-to-end validation |

## Arduino IDE Workflow

### Setting Up Test Environment

1. **Include test framework in your sketch:**
```cpp
#include "src/testing/TestFramework.h"
#include "src/testing/PowerOnSelfTest.h"
#include "src/ErrorSystem.h"
#include "src/PerformanceMonitor.h"

// Include test files
#include "test/test_stepper_driver.cpp"
#include "test/test_servo_driver.cpp" 
#include "test/test_terrapen_robot.cpp"
```

2. **Create a test runner sketch:**
```cpp
// File: examples/TestRunner/TestRunner.ino
#include <TerraPenMotionControl.h>
#include "src/testing/TestFramework.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  // Initialize test framework
  test_framework_init(true);  // verbose output
  
  // Run tests based on Serial input
  Serial.println("TerraPen Test Runner");
  Serial.println("Commands:");
  Serial.println("  'p' - Run POST");
  Serial.println("  'u' - Run Unit Tests");
  Serial.println("  'h' - Run Hardware Tests");
  Serial.println("  'a' - Run All Tests");
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    
    switch (cmd) {
      case 'p':
        Serial.println("Running POST...");
        robot_post();
        break;
        
      case 'u':
        Serial.println("Running Unit Tests...");
        test_set_mode(TEST_MODE_LOGIC);
        test_run_all();
        break;
        
      case 'h':
        Serial.println("Running Hardware Tests...");
        test_set_mode(TEST_MODE_HARDWARE);
        test_run_all();
        break;
        
      case 'a':
        Serial.println("Running All Tests...");
        test_set_mode(TEST_MODE_ALL);
        test_run_all();
        break;
    }
  }
  
  delay(100);
}
```

### Development Workflow

**Step 1: Start with POST**
```cpp
void setup() {
  Serial.begin(115200);
  
  // Run POST first
  PostResults result = robot_post();
  if (!result.passed) {
    Serial.println("POST failed - fix hardware before proceeding");
    while(1) delay(1000);  // Stop execution
  }
  
  // Continue with normal setup...
}
```

**Step 2: Development with Performance Monitoring**
```cpp
void loop() {
  PERF_START_LOOP();
  
  // Your main robot code
  PERF_START_UPDATE();
  robot.update();
  PERF_END_UPDATE();
  
  // Other loop code...
  
  PERF_END_LOOP();
  
  // Print performance summary every 10 seconds
  static unsigned long last_perf_report = 0;
  if (millis() - last_perf_report > 10000) {
    g_performance_monitor.printSummary();
    last_perf_report = millis();
  }
}
```

**Step 3: Pre-commit Testing**
```cpp
// Create a comprehensive test sketch for pre-commit validation
void runPreCommitTests() {
  Serial.println("=== PRE-COMMIT TEST SUITE ===");
  
  // 1. POST
  PostResults post = robot_post();
  if (!post.passed) {
    Serial.println("❌ POST FAILED - DO NOT COMMIT");
    return;
  }
  
  // 2. Unit Tests
  test_set_mode(TEST_MODE_ALL);
  test_run_all();
  
  if (test_all_passed()) {
    Serial.println("✅ ALL TESTS PASSED - SAFE TO COMMIT");
  } else {
    Serial.println("❌ TESTS FAILED - FIX BEFORE COMMITTING");
  }
}
```

## Command Line Workflow (Advanced)

For developers who prefer command-line testing:

### Using arduino-cli

1. **Install arduino-cli:**
```bash
# Windows (PowerShell)
Invoke-WebRequest -Uri "https://downloads.arduino.cc/arduino-cli/arduino-cli_latest_Windows_64bit.zip" -OutFile "arduino-cli.zip"
Expand-Archive arduino-cli.zip
Move-Item arduino-cli\arduino-cli.exe C:\arduino-cli\
$env:PATH += ";C:\arduino-cli"
```

2. **Create test automation script:**
```bash
# File: scripts/run_tests.ps1
param(
    [Parameter(Mandatory=$false)]
    [ValidateSet("post", "unit", "hardware", "all")]
    [string]$TestType = "all",
    
    [Parameter(Mandatory=$false)]
    [string]$Port = "COM3"
)

Write-Host "Running TerraPen tests: $TestType"

# Compile and upload test runner
arduino-cli compile --fqbn arduino:avr:nano examples/TestRunner
arduino-cli upload --fqbn arduino:avr:nano --port $Port examples/TestRunner

# Send test command and capture results
Start-Sleep 2
$testCommand = switch ($TestType) {
    "post" { "p" }
    "unit" { "u" }
    "hardware" { "h" }
    "all" { "a" }
}

# Send command via serial and capture output
# (Implementation depends on your serial tool preference)
Write-Host "Test command sent: $testCommand"
```

3. **Git pre-commit hook:**
```bash
# File: .git/hooks/pre-commit
#!/bin/bash
echo "Running TerraPen pre-commit tests..."

# Run unit tests (no hardware required)
powershell -File scripts/run_tests.ps1 -TestType unit

if [ $? -ne 0 ]; then
    echo "❌ Unit tests failed - commit aborted"
    exit 1
fi

echo "✅ Tests passed - proceeding with commit"
```

## Continuous Integration Setup

### GitHub Actions Workflow

```yaml
# File: .github/workflows/arduino-tests.yml
name: Arduino Tests

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  compile-tests:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup Arduino CLI
      uses: arduino/setup-arduino-cli@v1
    
    - name: Install Arduino cores
      run: |
        arduino-cli core update-index
        arduino-cli core install arduino:avr
    
    - name: Install dependencies
      run: |
        arduino-cli lib install Servo
    
    - name: Compile Library Examples
      run: |
        arduino-cli compile --fqbn arduino:avr:nano examples/BasicMovement
        arduino-cli compile --fqbn arduino:avr:nano examples/Phase1_5Integration
        arduino-cli compile --fqbn arduino:avr:nano examples/TestRunner
    
    - name: Compile Unit Tests
      run: |
        arduino-cli compile --fqbn arduino:avr:nano examples/TestRunner
    
    - name: Check code format
      run: |
        # Add code formatting checks if desired
        echo "Code format check passed"
    
  syntax-validation:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Validate syntax
      run: |
        # Check for common syntax issues
        find . -name "*.cpp" -o -name "*.h" | xargs grep -l "Serial.print" | wc -l
        echo "Syntax validation completed"
```

### Hardware-in-the-Loop (HIL) Testing

For organizations with dedicated test hardware:

```yaml
# File: .github/workflows/hardware-tests.yml
name: Hardware Tests

on:
  workflow_run:
    workflows: ["Arduino Tests"]
    types:
      - completed

jobs:
  hardware-tests:
    runs-on: self-hosted  # Requires self-hosted runner with Arduino hardware
    if: ${{ github.event.workflow_run.conclusion == 'success' }}
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Flash test firmware
      run: |
        arduino-cli upload --fqbn arduino:avr:nano --port /dev/ttyUSB0 examples/TestRunner
    
    - name: Run hardware tests
      run: |
        python scripts/hardware_test_runner.py --port /dev/ttyUSB0 --timeout 300
    
    - name: Archive test results
      uses: actions/upload-artifact@v3
      with:
        name: hardware-test-results
        path: test-results.json
```

## Test Result Analysis

### Understanding Test Output

**Successful Test Run:**
```
=== TerraPen Unit Testing Framework ===
Test mode: ALL TESTS

Running: StepperDriver_Initialization... PASS
Running: StepperDriver_TimingAccuracy... PASS
Running: ServoDriver_Initialization... PASS
...

=================================
         TEST SUMMARY
=================================
Total Tests:    42
Passed:         42
Failed:         0
Success Rate:   100.0%
Execution Time: 1547 ms

✓ ALL TESTS PASSED!
=================================
```

**Failed Test Run:**
```
=== TerraPen Unit Testing Framework ===
Test mode: ALL TESTS

Running: StepperDriver_Initialization... PASS
Running: StepperDriver_TimingAccuracy... FAIL
FAIL: Hardware Tests > StepperDriver_TimingAccuracy - first_step at line 89

=================================
         TEST SUMMARY
=================================
Total Tests:    42
Passed:         41
Failed:         1
Success Rate:   97.6%
Execution Time: 1203 ms

✗ SOME TESTS FAILED!
Failed tests: 1
=================================
```

### Performance Monitoring Output

```
=== Performance Summary ===
Update Frequency: 987.3 Hz (target: 1000 Hz)
CPU Utilization: 23.4%
Update Time: 124 μs avg, 230 μs max
Free Memory: 1247 bytes
===========================
```

**What to Look For:**
- **Update Frequency** should be close to target (within 10%)
- **CPU Utilization** should be <80% for safety margin
- **Free Memory** should remain stable (not decreasing over time)
- **No missed steps or timing violations**

## Troubleshooting Common Issues

### Test Framework Issues

**Problem:** Tests hang or never complete
**Solution:** 
- Check hardware connections
- Increase timeout values in test configuration
- Run tests individually to isolate the problem

**Problem:** Random test failures
**Solution:**
- Check power supply stability
- Add delays between hardware operations
- Review timing tolerances in tests

**Problem:** Memory-related crashes
**Solution:**
- Monitor free memory during tests
- Reduce test history buffer sizes
- Check for memory leaks in test code

### Performance Issues

**Problem:** Low update frequency
**Solution:**
- Optimize update() function
- Reduce Serial output during operation
- Check for blocking operations

**Problem:** High CPU utilization
**Solution:**
- Profile individual functions
- Optimize timing-critical code paths
- Consider reducing update frequency

## Integration with IDEs

### PlatformIO Integration

```ini
# File: platformio.ini
[env:test]
platform = atmelavr
board = nanoatmega328
framework = arduino

# Test configuration
test_framework = custom
test_build_project_src = true

# Custom test flags
build_flags = 
    -DTEST_MODE=1
    -DVERBOSE_TESTING=1

# Test dependencies
lib_deps = 
    Servo

# Upload settings for testing
upload_port = COM3
monitor_port = COM3
monitor_speed = 115200
```

### VSCode Extensions

Recommended extensions for testing workflow:
- **Arduino** (Microsoft) - Core Arduino support
- **Serial Monitor** - Real-time test output
- **GitLens** - Pre-commit validation
- **Test Explorer** - Visual test management

This testing framework provides comprehensive validation while being lightweight enough for embedded systems. The key is to run tests regularly and address issues immediately rather than letting them accumulate.