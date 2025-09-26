# TerraPen Testing

## Quick Test Commands

### Math Validation (No Hardware Required)

**Windows:**
```bash
./test_runner.bat
```

**Command Line:**
```bash
pio run -e test-math
```

**What it does:** Validates all coordinate mathematics, differential drive algorithms, and robot control logic without requiring any Arduino hardware.

**Output:** Build success/failure and comprehensive mathematical validation.

**Time:** ~1-2 seconds

## Test Output

### Success:
```
==================================== [SUCCESS] Took 1.31 seconds ====================================

Environment    Status    Duration
-------------  --------  ------------
test-math      SUCCESS   00:00:01.308
==================================== 1 succeeded in 00:00:01.308 ====================================
```

### What Gets Tested:
- Position coordinate calculations (x, y, heading)
- Distance and angle mathematics
- Differential drive kinematics
- Movement path planning  
- Angle normalization
- Position interpolation
- Edge cases and precision

## Development Workflow

1. Make changes to robot algorithms
2. Run `./test_runner.bat` or `pio run -e test-math`
3. Look for `[SUCCESS]` in output
4. If successful: algorithms are mathematically sound
5. If failed: fix errors before continuing

## Memory Warning (Normal)

You'll see a memory warning - this is completely normal for comprehensive tests:
```
Warning! The data size (2946 bytes) is greater than maximum allowed (2048 bytes)
```

The actual robot code uses much less memory. This warning only affects the testing build.