# TerraPen Motion System Testing

## Multi-Device Testing Strategy

The TerraPen system consists of three components that can be tested independently:

1. **Arduino Nano Firmware** - Mathematical validation and hardware integration
2. **ESP32 Controller** - WiFi communication and web interface  
3. **Desktop GUI** - Python interface and robot simulation

## Quick Test Commands

### Arduino Nano Tests (No Hardware Required)

**Windows:**
```bash
cd nano-firmware
./test_runner.bat
```

**Cross-platform:**
```bash
cd nano-firmware
pio run -e test-math
```

**What it does:** Validates all coordinate mathematics, differential drive algorithms, and robot control logic without requiring any Arduino hardware.

**Output:** Build success/failure and comprehensive mathematical validation.

**Time:** ~1-2 seconds

### ESP32 Controller Tests

**Build Validation:**
```bash
cd esp32-controller
pio run
```

**Upload and Test:**
```bash
cd esp32-controller  
pio run -t upload
# Connect to WiFi AP "TerraPen-Robot" and visit http://192.168.4.1
```

### Desktop GUI Tests

**Install and Run:**
```bash
cd desktop-gui
pip install -r requirements.txt
python main.py
```

**Simulator Mode:**
```bash
cd desktop-gui/robot_simulator
python robot_tracker_gui.py
```

## Test Output

## Test Output Examples

### Arduino Nano Math Validation Success

```text
==================================== [SUCCESS] Took 1.31 seconds ====================================

Environment    Status    Duration
-------------  --------  ------------
test-math      SUCCESS   00:00:01.308
==================================== 1 succeeded in 00:00:01.308 ====================================
```

### ESP32 Controller Build Success

```text
RAM:   [==        ]  18.2% (used 59564 bytes from 327680 bytes)
Flash: [========= ]  94.1% (used 1233845 bytes from 1310720 bytes)
```

### What Gets Tested

**Arduino Nano Firmware:**

- Position coordinate calculations (x, y, heading)
- Distance and angle mathematics
- Differential drive kinematics
- Movement path planning  
- Angle normalization
- Position interpolation
- Edge cases and precision

**ESP32 Controller:**

- WiFi AP initialization and client connections
- Web server HTTP request handling
- UART communication with Arduino Nano
- JSON protocol parsing and response generation

**Desktop GUI:**

- Robot simulator mathematical accuracy
- WiFi client connection and communication
- GUI widget responsiveness and data display

## Multi-Device Development Workflow

### 1. Arduino Nano Development

```bash
cd nano-firmware
pio run -e test-math          # Validate mathematics
pio run -t upload             # Upload to hardware
```

### 2. ESP32 Controller Development

```bash
cd esp32-controller
pio run                       # Build and validate
pio run -t upload             # Deploy wireless controller
```

### 3. Desktop GUI Development

```bash
cd desktop-gui
python -m pytest tests/      # Run Python unit tests
python main.py               # Test full GUI
```

### 4. System Integration Testing

1. Upload both firmwares (Nano + ESP32)
2. Connect to "TerraPen-Robot" WiFi AP
3. Launch desktop GUI and connect to robot
4. Test end-to-end movement commands

## Memory Usage Notes

**Arduino Nano Warning (Normal):**

```text
Warning! The data size (2946 bytes) is greater than maximum allowed (2048 bytes)
```

This warning appears during comprehensive testing and is expected. The actual robot firmware uses much less memory. This warning only affects the testing build with extensive validation code.

**ESP32 Controller:**

The ESP32 has abundant memory (327KB RAM, 1.3MB Flash) and should not show memory warnings under normal operation.