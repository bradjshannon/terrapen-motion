# ESP32 Controller Documentation

## Overview

The ESP32 Controller serves as a wireless bridge between the desktop GUI and Arduino Nano firmware. It provides a WiFi Access Point for robot communication and a web interface for basic control.

## Hardware Requirements

- ESP32-S3 Zero (or compatible ESP32-S3 board)
- USB-C connection for programming and power
- Connection to Arduino Nano via UART (pins 44/43)

## Features

- **WiFi Access Point**: Creates "TerraPen-Robot" network (192.168.4.1)
- **Web Interface**: Browser-based robot control at http://192.168.4.1
- **UART Bridge**: Translates WiFi commands to Arduino Nano JSON protocol
- **OTA Updates**: Over-the-air firmware updates for Arduino Nano
- **Status Monitoring**: Real-time robot status and position feedback

## Configuration

### WiFi Settings

```cpp
const char* ssid = "TerraPen-Robot";
const char* password = "terrapen123";
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);
```

### UART Communication

```cpp
#define NANO_RX_PIN 44  // ESP32 TX -> Arduino RX
#define NANO_TX_PIN 43  // ESP32 RX -> Arduino TX
#define UART_BAUD 9600
```

## API Endpoints

### Web Interface

- `GET /` - Main control interface (HTML)
- `GET /status` - Robot status (JSON)
- `POST /command` - Send robot command (JSON)

### Command Format

Send POST requests to `/command` with JSON payload:

```json
{
  "cmd": 1,
  "x": 50.0,
  "y": 30.0,
  "pen_down": false
}
```

### Response Format

Receive JSON responses with robot status:

```json
{
  "position": {
    "x": 25.5,
    "y": 15.2,
    "angle": 1.57
  },
  "status": "moving",
  "error": 0
}
```

## Building and Deployment

### Build

```bash
cd esp32-controller
pio run
```

### Upload to ESP32

```bash
pio run -t upload
```

### Monitor Serial Output

```bash
pio device monitor
```

## Usage

1. **Upload Firmware**: Deploy to ESP32-S3 board
2. **Power On**: ESP32 creates WiFi access point
3. **Connect**: Join "TerraPen-Robot" WiFi network (password: terrapen123)
4. **Web Interface**: Visit http://192.168.4.1 in browser
5. **Desktop GUI**: Use desktop application to connect to same IP

## Troubleshooting

### Build Issues

- Ensure ESP32-S3 board support is installed in PlatformIO
- Check that ArduinoJson library dependencies are resolved
- Verify pin assignments match your ESP32-S3 board variant

### WiFi Connection Issues

- Check that ESP32 is powered and firmware is uploaded
- Verify WiFi network "TerraPen-Robot" appears in available networks
- Ensure computer WiFi is connected to robot network, not internet

### UART Communication Issues

- Verify physical connections between ESP32 pins 44/43 and Arduino Nano
- Check baud rate matches (9600) on both devices
- Monitor serial output to debug JSON protocol parsing

## Development Notes

- Web interface uses embedded HTML/CSS/JavaScript in main.cpp
- JSON parsing handled by ArduinoJson library (v7.0.0)
- UART communication is non-blocking to prevent web server delays
- Built-in LED indicates WiFi and communication status

## Future Enhancements

- WebSocket support for real-time position updates
- Bluetooth Low Energy (BLE) alternative to WiFi
- Configuration web page for WiFi credentials
- Firmware update interface for Arduino Nano OTA updates
- Status dashboard with movement history and performance metrics