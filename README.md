# TerraPen Motion System

A complete differential drive robot system with pen control for drawing applications. Features multi-device architecture with Arduino Nano for real-time control, ESP32 for wireless communication, and desktop GUI for user interaction.

## System Architecture

```
Desktop GUI ←─WiFi/BLE─→ ESP32 Controller ←─UART─→ Arduino Nano ←─Hardware─→ Motors & Sensors
```

## Project Structure (Restructured)

```
terrapen-motion/
├── nano-firmware/           # Arduino Nano control firmware (PlatformIO)
│   ├── src/                 # Complete Phase 2 robot control library
│   ├── examples/            # Hardware and integration examples  
│   ├── test/                # Comprehensive test suite
│   └── library.properties   # Arduino library metadata
├── esp32-controller/        # ESP32-S3 wireless controller (PlatformIO)
│   ├── src/                 # WiFi/BLE bridge and web interface
│   └── scripts/             # Build automation and firmware embedding
├── desktop-gui/             # Desktop control application (Python)
│   ├── src/gui/             # Tkinter interface
│   ├── src/communication/   # WiFi/BLE client
│   └── robot_simulator/     # Moved from root - simulation tools
├── shared/                  # Common protocols and definitions
│   └── protocols/           # Communication protocol specifications
└── docs/                    # System documentation
```

## Quick Start

### 1. Arduino Nano Firmware
```bash
cd nano-firmware
pio run -t upload  # Build and upload to Nano
```

### 2. ESP32 Controller  
```bash
cd esp32-controller
pio run -t upload  # Build and upload to ESP32
```

### 3. Desktop GUI
```bash
cd desktop-gui
pip install -r requirements.txt
python main.py
```

## Development Status

✅ **Phase 1.5 Complete**: Step-based movement coordination  
✅ **Phase 2 Complete**: Full coordinate system with differential drive kinematics   
🔄 **Phase 3 Active**: Multi-device communication and wireless control

### Current Capabilities (Nano Firmware)
- Precise coordinate-based movement (`moveTo(x, y)`, `drawTo(x, y)`)
- Differential drive kinematics with position tracking
- Comprehensive error handling (60+ error codes)
- Real-time performance monitoring
- NVRAM configuration and data logging
- Extensive test coverage (math validation + hardware integration)

### New in Phase 3 (ESP32 + Desktop)
- Wireless robot control via WiFi
- Web-based interface accessible from any device
- Desktop GUI with drawing canvas and real-time status
- Over-the-air firmware updates for Arduino Nano
- Coordinated multi-device build system

## Hardware Configuration

- **Arduino Nano**: Real-time motor control and sensor processing
  - Two 28BYJ stepper motors via ULN2803A driver
  - Servo for pen up/down control
  - Photodiode for line detection
  - Obstacle detection switches
  
- **ESP32-S3 Zero**: Wireless communication and coordination
  - WiFi Access Point mode (192.168.4.1)
  - UART communication with Nano (pins 44/43)
  - Web interface for basic control
  - Over-the-air Nano firmware updates

- **Physical Specs**:
  - Wheel diameter: 25mm
  - Wheelbase: 30mm (differential drive)
  - Coordinate system: Phase 2 complete with full kinematics

## Communication Protocol

The system uses a structured JSON-based protocol over UART between ESP32 and Nano:

```json
// Command: Move to coordinate
{"cmd": 1, "x": 50.0, "y": 30.0, "pen_down": false}

// Response: Position update  
{"position": {"x": 25.5, "y": 15.2, "angle": 1.57}, "status": "moving"}
```

See `shared/protocols/nano_uart_protocol.json` for complete specification.

## Building and Testing

### Complete System Test (Recommended)

```bash
# Test all components at once
./test_all.bat          # Windows
./test_all.sh           # Linux/macOS
```

### Individual Component Testing

**Arduino Nano Firmware:**
```bash
cd nano-firmware
./test_runner.bat       # Windows - Math validation (no hardware)
pio run -e test-math    # Cross-platform alternative
pio run -t upload       # Deploy to hardware
```

**ESP32 Controller:**
```bash
cd esp32-controller
./test_runner.bat       # Windows - Build validation
pio run                 # Cross-platform alternative  
pio run -t upload       # Deploy wireless bridge
```

**Desktop GUI:**
```bash
cd desktop-gui
./test_runner.bat       # Windows - Dependency validation
pip install -r requirements.txt  # Install dependencies
python main.py          # Launch control interface
```

### Hardware Integration Testing

```bash
# After uploading both firmwares
cd nano-firmware
pio run -e test-integration -t upload  # Hardware-dependent tests
```

## VS Code Integration

The workspace is configured for multi-project PlatformIO development with proper IntelliSense, debugging, and task automation. Both Arduino and ESP32 projects are automatically detected.

## Previous Implementation (See README_backup.md)

The original single-project Arduino library remains fully functional in `nano-firmware/`. All Phase 1.5 and Phase 2 features are preserved and enhanced. This restructure adds the multi-device communication layer while maintaining the robust foundation.

For detailed documentation of the Arduino library features, testing framework, and Phase 2 coordinate system implementation, see `README_backup.md`.