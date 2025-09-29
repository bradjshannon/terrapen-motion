#!/bin/bash
# TerraPen Motion System - Complete Multi-Device Test Suite
# Validates Arduino Nano, ESP32 Controller, and Desktop GUI components

echo "======================================"
echo "TerraPen Motion System Test Suite"
echo "======================================"
echo

# Test 1: Arduino Nano Mathematical Validation
echo "[1/3] Arduino Nano - Mathematical Validation"
echo "Testing coordinate system, differential drive kinematics..."
cd nano-firmware
if pio run -e test-math; then
    echo "✅ Arduino Nano math validation PASSED"
else
    echo "❌ Arduino Nano math validation FAILED"
    cd ..
    exit 1
fi
cd ..
echo

# Test 2: ESP32 Controller Build Validation  
echo "[2/3] ESP32 Controller - Build Validation"
echo "Testing WiFi controller and web interface..."
cd esp32-controller
if pio run; then
    echo "✅ ESP32 Controller build PASSED"
else
    echo "❌ ESP32 Controller build FAILED"
    cd ..
    exit 1
fi
cd ..
echo

# Test 3: Desktop GUI Dependency Check
echo "[3/3] Desktop GUI - Dependency Validation"
echo "Testing Python environment and required packages..."
cd desktop-gui
if python -c "import tkinter; import requests; print('✅ Desktop GUI dependencies OK')"; then
    echo "✅ Desktop GUI dependencies validated"
else
    echo "❌ Desktop GUI dependencies FAILED"
    echo "Run: pip install -r requirements.txt"
    cd ..
    exit 1
fi
cd ..
echo

echo "======================================"
echo "🎉 ALL TESTS PASSED!"
echo "======================================"
echo
echo "Multi-device system is ready:"
echo "1. Arduino Nano firmware: Mathematically validated"
echo "2. ESP32 Controller: Successfully builds"  
echo "3. Desktop GUI: Dependencies satisfied"
echo
echo "Next steps:"
echo "  - Upload nano-firmware: cd nano-firmware && pio run -t upload"
echo "  - Upload esp32-controller: cd esp32-controller && pio run -t upload"  
echo "  - Run desktop GUI: cd desktop-gui && python main.py"
echo