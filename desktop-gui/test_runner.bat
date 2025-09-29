@echo off
REM Desktop GUI - TerraPen Control Interface Validation
REM Tests Python dependencies and GUI functionality

echo ======================================
echo TerraPen Motion - Desktop GUI Tests
echo ======================================
echo Testing desktop control interface...
echo.

echo Checking Python dependencies...
python -c "import tkinter; print('✅ tkinter: OK')"
if errorlevel 1 goto :dep_error

python -c "import requests; print('✅ requests: OK')"
if errorlevel 1 goto :dep_error

echo.
echo Testing GUI components...
python -c "
import sys
import os
sys.path.insert(0, os.path.join('.', 'src'))

try:
    from communication.wifi_client import WiFiClient
    print('✅ WiFi client: OK')
except ImportError as e:
    print('❌ WiFi client import failed:', e)
    sys.exit(1)

try:
    from gui.main_window import MainWindow
    print('✅ Main window: OK')
except ImportError as e:
    print('❌ Main window import failed:', e)
    sys.exit(1)

print('✅ All GUI components validated')
"

if errorlevel 1 (
    echo.
    echo ❌ Desktop GUI validation FAILED
    pause
    exit /b 1
) else (
    echo.
    echo ✅ Desktop GUI validation PASSED
    echo Control interface is ready.
    echo.
    echo Next steps:
    echo   - Run GUI: python main.py
    echo   - Run simulator: cd robot_simulator ^&^& python robot_tracker_gui.py
    pause
)
goto :end

:dep_error
echo.
echo ❌ Missing Python dependencies
echo Please install: pip install -r requirements.txt
pause
exit /b 1

:end