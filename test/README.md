# TerraPen Motion Control Library Tests

This directory contains **focused, meaningful tests** for the TerraPen Motion Control Arduino library.

## Philosophy

These tests focus on the **core algorithms and mathematical calculations** that are:
- Complex and error-prone
- Critical for robot functionality  
- Benefit from automated verification
- Independent of hardware specifics

## Test Structure

### Mathematical Tests (No Arduino Required)

✅ **Mathematical Validation Test** - Complete validation of Position class coordinate mathematics

- **Location**: `src/MathValidationMain.cpp` (enabled with `MATH_VALIDATION_MODE`)
- **Run with**: `pio run -e test-math` (compiles without uploading)
- **Upload and run**: `pio run -e test-math --upload-port COM<X>` (see serial output at 9600 baud)

### Hardware Integration Tests (Arduino Required)

- **Hardware integration tests** exist in `test/` directory but require actual Arduino hardware

## Mathematical Validation Test Coverage

The mathematical validation test validates 20+ critical operations:

✅ **Position Coordinate Mathematics**

- Distance calculations (Pythagorean theorem)
- Zero distance validation
- Cardinal direction angle calculations (North=90°, East=0°)

✅ **Position Creation and Equality**

- Position constructor validation
- Component access (x, y, angle)
- Position equality comparison with custom logic

✅ **Position Interpolation**

- Linear interpolation of coordinates
- Midpoint and quarter-point calculations
- Boundary conditions (t=0, t=1)

✅ **Angle Normalization**

- Multi-revolution angle normalization to [-π, π] range
- Positive and negative angle wrapping
- Edge case handling (3π → π, -3π → -π)

✅ **Polar Coordinate Creation**

- Distance/angle to X/Y conversion
- Trigonometric calculations validation
- Static factory method verification

✅ **Movement Calculations**

- Distance and bearing calculations for target positions
- Cardinal movement validation (East, diagonal)
- Complex movement mathematics

✅ **Edge Cases and Precision**

- Small movement precision (0.1mm accuracy)
- Tolerance-based equality checking
- Boundary and precision validation

## What We Test (Meaningful Only)

✅ **Complex Mathematical Operations** that are error-prone and critical

✅ **Coordinate System Foundations** that all robot movement depends on

✅ **Algorithms** that involve floating-point precision and trigonometry

✅ **Edge Cases** where mathematical operations can fail or produce unexpected results

## What We DON'T Test

❌ Simple getters/setters
❌ Arduino framework functions  
❌ Hardware I/O (except integration tests)
❌ Trivial operations that can't meaningfully fail

## Running Tests

```bash
# Run mathematical validation (compile only, no hardware needed)
pio run -e test-math

# Upload mathematical validation to Arduino and see results via Serial Monitor
pio run -e test-math --upload-port COM3

# Verify build size and memory usage
pio run -e test-math --target checkprogsize

# Run hardware integration tests (Arduino + motors required)
pio test -e test-integration --upload-port COM3
```

## Expected Output

The mathematical validation test outputs:
- Individual test results (✓ PASS / ✗ FAIL)
- Overall success rate percentage  
- Clear indication if core algorithms are working
- Summary of validated mathematical foundations

Example output:
```
=== TerraPen Motion Control - Mathematical Validation ===
Testing core position algorithms without hardware dependency

--- Position Coordinate Mathematics ---
Test: Distance calculation (3,4,5 triangle) ... ✓ PASS
Test: Zero distance to self ... ✓ PASS
...

MATHEMATICAL VALIDATION COMPLETE
Tests passed: 20 / 20
Success rate: 100.0%

🎉 ALL TESTS PASSED!
Core mathematical algorithms are working correctly.
Position coordinate system is ready for robot control.
```

## Integration Status

✅ **All meaningful mathematical tests are now integrated** into the project and can run without physical Arduino hardware.

The mathematical validation test covers all critical coordinate system algorithms that form the foundation of robot movement control. This validates the library's core mathematical correctness before any hardware interaction.