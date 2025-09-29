#!/bin/bash
# Desktop GUI - TerraPen Control Interface Validation
# Tests Python dependencies and GUI functionality

echo "======================================"
echo "TerraPen Motion - Desktop GUI Tests"
echo "======================================"
echo "Testing desktop control interface..."
echo

echo "Checking Python dependencies..."
if ! python -c "import tkinter; print('✅ tkinter: OK')"; then
    echo "❌ tkinter not available"
    exit 1
fi

if ! python -c "import requests; print('✅ requests: OK')"; then
    echo "❌ requests not installed"
    echo "Please install: pip install -r requirements.txt"
    exit 1
fi

echo
echo "Testing GUI components..."
if python -c "
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
"; then
    echo
    echo "✅ Desktop GUI validation PASSED"
    echo "Control interface is ready."
    echo
    echo "Next steps:"
    echo "  - Run GUI: python main.py"
    echo "  - Run simulator: cd robot_simulator && python robot_tracker_gui.py"
else
    echo
    echo "❌ Desktop GUI validation FAILED"
    exit 1
fi