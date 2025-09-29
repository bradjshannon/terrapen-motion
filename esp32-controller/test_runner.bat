@echo off
REM ESP32 Controller - TerraPen WiFi Bridge Build Validation
REM Tests wireless controller compilation and integration

echo ======================================
echo TerraPen Motion - ESP32 Controller Tests
echo ======================================
echo Testing ESP32 WiFi bridge and web interface...
echo.

call pio run

if errorlevel 1 (
    echo.
    echo ❌ ESP32 Controller build FAILED
    echo Please check WiFi and web server implementation.
    pause
    exit /b 1
) else (
    echo.
    echo ✅ ESP32 Controller build PASSED
    echo WiFi bridge is ready for deployment.
    echo.
    echo Next steps:
    echo   - Upload: pio run -t upload
    echo   - Connect to WiFi AP "TerraPen-Robot" 
    echo   - Visit: http://192.168.4.1
    pause
)