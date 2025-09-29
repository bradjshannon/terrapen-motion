# Desktop GUI Documentation

## Overview

The Desktop GUI provides a comprehensive control interface for the TerraPen robot system. It connects to the ESP32 controller via WiFi and offers both manual control and automated drawing capabilities.

## Features

- **Robot Control**: Direct movement commands (move, draw, pen control)
- **Drawing Canvas**: Visual representation of robot position and drawing
- **WiFi Communication**: Connects to ESP32 controller over WiFi
- **Real-time Status**: Live position, battery, and error monitoring  
- **Pattern Generator**: Automated drawing of geometric shapes and patterns
- **Robot Simulator**: Test interface without hardware (offline mode)

## Installation

### Prerequisites

- Python 3.7 or higher
- pip package manager
- WiFi capability for robot connection

### Setup

```bash
cd desktop-gui
pip install -r requirements.txt
```

### Required Packages

```text
tkinter          # GUI framework (built-in with Python)
requests         # HTTP communication with ESP32
matplotlib       # Drawing canvas and visualization
numpy            # Mathematical calculations
configparser     # Configuration file management
```

## Usage

### Launch Main Application

```bash
cd desktop-gui
python main.py
```

### Launch Simulator (Offline Mode)

```bash
cd desktop-gui/robot_simulator
python robot_tracker_gui.py
```

## GUI Components

### Main Control Panel

- **Connection Status**: WiFi connection indicator
- **Position Display**: Current robot coordinates (x, y, angle)
- **Movement Controls**: Arrow keys and distance input
- **Pen Controls**: Pen up/down toggle
- **Emergency Stop**: Immediate halt button

### Drawing Canvas

- **Robot Position**: Real-time position indicator
- **Drawing Path**: Visual trace of pen-down movements
- **Grid Lines**: Coordinate reference grid
- **Zoom/Pan**: Canvas navigation controls

### Pattern Generator

- **Geometric Shapes**: Squares, circles, triangles
- **Custom Paths**: Import SVG or coordinate sequences
- **Preview Mode**: Visualize before execution
- **Speed Control**: Adjust movement and drawing speed

## Configuration

### Robot Connection Settings

Edit `robot_config.ini`:

```ini
[robot]
ip_address = 192.168.4.1
port = 80
timeout = 5.0
retry_attempts = 3

[drawing]
canvas_width = 800
canvas_height = 600
grid_size = 10
default_speed = 50
```

### WiFi Setup

1. Connect computer to "TerraPen-Robot" WiFi network
2. Password: `terrapen123`
3. Launch desktop GUI
4. GUI automatically connects to robot at 192.168.4.1

## API Integration

### Robot Communication

The desktop GUI communicates with the ESP32 controller using HTTP requests:

```python
# Send movement command
response = requests.post('http://192.168.4.1/command', json={
    "cmd": 1,
    "x": 50.0,
    "y": 30.0,
    "pen_down": True
})

# Get robot status
status = requests.get('http://192.168.4.1/status').json()
print(f"Position: ({status['position']['x']}, {status['position']['y']})")
```

### Command Types

1. **Move Command** (`cmd: 1`): Move to coordinate
2. **Draw Command** (`cmd: 2`): Draw line to coordinate  
3. **Pen Command** (`cmd: 3`): Control pen up/down
4. **Stop Command** (`cmd: 4`): Emergency stop
5. **Status Query** (`cmd: 5`): Get current status

## Robot Simulator

The simulator provides offline testing without hardware requirements:

### Features

- **Mathematical Validation**: Test coordinate calculations
- **Pattern Preview**: Visualize drawing patterns before execution
- **Performance Testing**: Measure drawing accuracy and timing
- **Development Mode**: Debug GUI components without robot

### Launch Simulator

```bash
cd desktop-gui/robot_simulator
python robot_tracker_gui.py
```

## Troubleshooting

### Connection Issues

**Cannot connect to robot:**
1. Verify ESP32 firmware is uploaded and running
2. Connect to "TerraPen-Robot" WiFi network
3. Check IP address is 192.168.4.1
4. Test connection: `ping 192.168.4.1`

**GUI shows "Disconnected":**
1. Check WiFi connection to robot network
2. Restart desktop GUI application
3. Verify ESP32 web server is responding

### GUI Issues

**Tkinter import error:**
```bash
# Ubuntu/Debian
sudo apt-get install python3-tk

# Windows - reinstall Python with tkinter option
# macOS - install Python from python.org
```

**Missing dependencies:**
```bash
cd desktop-gui
pip install -r requirements.txt
```

## Development

### Project Structure

```
desktop-gui/
├── main.py                    # Main application entry point
├── requirements.txt           # Python dependencies
├── src/
│   ├── gui/
│   │   └── main_window.py     # Primary GUI interface
│   └── communication/
│       └── wifi_client.py     # ESP32 communication client
└── robot_simulator/
    ├── robot_simulator.py     # Simulation engine
    ├── robot_tracker_gui.py   # Simulator GUI
    └── demo_patterns.py       # Example drawing patterns
```

### Adding New Features

1. **New GUI Controls**: Edit `src/gui/main_window.py`
2. **Communication Protocol**: Modify `src/communication/wifi_client.py`
3. **Drawing Patterns**: Add to `robot_simulator/demo_patterns.py`
4. **Configuration Options**: Update `robot_config.ini`

### Testing

```bash
# Validate dependencies and imports
cd desktop-gui
./test_runner.bat          # Windows
./test_runner.sh           # Linux/macOS

# Run with simulator for offline testing  
python robot_simulator/robot_tracker_gui.py
```

## Future Enhancements

- **Advanced Drawing Tools**: Bezier curves, text rendering
- **Multi-robot Support**: Control multiple robots simultaneously
- **Drawing Import/Export**: SVG file support, G-code compatibility
- **Remote Access**: Web-based interface accessible from mobile devices
- **Recording/Playback**: Save and replay drawing sequences
- **Performance Analytics**: Movement accuracy and timing analysis