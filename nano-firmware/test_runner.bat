@echo off
REM Arduino Nano - TerraPen Motion Mathematical Validation
REM Tests core coordinate algorithms without hardware dependency

echo ======================================
echo TerraPen Motion - Arduino Nano Tests
echo ======================================
echo Testing coordinate system mathematics without requiring hardware...
echo.

call pio run -e test-math

if errorlevel 1 (
    echo.
    echo ❌ Mathematical validation FAILED
    echo Please check coordinate system implementation.
    pause
    exit /b 1
) else (
    echo.
    echo ✅ Mathematical validation PASSED
    echo Core algorithms are working correctly.
    echo Arduino Nano firmware is ready for hardware deployment.
    pause
)