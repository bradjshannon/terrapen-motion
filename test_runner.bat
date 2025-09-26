@echo off
echo ========================================
echo TerraPen Motion Control - Test Runner
echo ========================================
echo.
echo Running mathematical validation tests...
echo (No hardware required)
echo.

pio run -e test-math

echo.
echo ========================================
if %errorlevel% equ 0 (
    echo ✅ TESTS COMPLETED SUCCESSFULLY
    echo All mathematical algorithms validated
) else (
    echo ❌ TESTS FAILED
    echo Check the output above for errors
)
echo ========================================
pause