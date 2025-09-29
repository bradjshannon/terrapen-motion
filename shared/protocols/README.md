# Communication Protocols

## Overview

The TerraPen system uses structured JSON-based protocols for communication between components:

- **WiFi Protocol**: Desktop GUI ↔ ESP32 Controller (HTTP/JSON)
- **UART Protocol**: ESP32 Controller ↔ Arduino Nano (Serial/JSON)

## WiFi Protocol (Desktop ↔ ESP32)

### Connection

- **Method**: HTTP POST/GET requests
- **Address**: 192.168.4.1:80
- **Network**: WiFi AP "TerraPen-Robot" (password: terrapen123)

### Endpoints

#### GET /status

Returns current robot status:

```json
{
  "position": {
    "x": 25.5,
    "y": 15.2,
    "angle": 1.57
  },
  "status": "moving",
  "error": 0,
  "pen_down": false,
  "battery_voltage": 7.4
}
```

#### POST /command  

Send robot commands:

```json
{
  "cmd": 1,
  "x": 50.0,
  "y": 30.0,
  "pen_down": true,
  "speed": 100
}
```

### Command Types

| Command | ID | Description | Required Parameters |
|---------|----|-----------  |-------------------- |
| Move    | 1  | Move to coordinate | x, y |
| Draw    | 2  | Draw line to coordinate | x, y |  
| Pen     | 3  | Control pen position | pen_down |
| Stop    | 4  | Emergency stop | (none) |
| Home    | 5  | Return to origin | (none) |
| Status  | 6  | Query status | (none) |

## UART Protocol (ESP32 ↔ Nano)

### Connection

- **Interface**: Hardware Serial (UART1)
- **Baud Rate**: 9600
- **Format**: 8N1 (8 data bits, no parity, 1 stop bit)
- **ESP32 Pins**: TX=44, RX=43
- **Arduino Pins**: TX=1, RX=0

### Message Format

All messages are JSON objects terminated with newline (`\n`):

```json
{"cmd": 1, "x": 50.0, "y": 30.0, "pen_down": false}
```

### Command Messages (ESP32 → Arduino)

#### Movement Commands

```json
// Move to absolute coordinate
{"cmd": 1, "x": 50.0, "y": 30.0}

// Draw line to coordinate (pen down)  
{"cmd": 2, "x": 100.0, "y": 75.0}

// Relative movement
{"cmd": 7, "dx": 10.0, "dy": -5.0}
```

#### Control Commands

```json
// Pen control
{"cmd": 3, "pen_down": true}

// Emergency stop
{"cmd": 4}

// Set movement speed (mm/s)
{"cmd": 8, "speed": 50}

// Home to origin
{"cmd": 5}
```

#### Configuration Commands

```json
// Set robot parameters
{"cmd": 9, "wheel_diameter": 25.0, "wheelbase": 30.0}

// Calibrate position
{"cmd": 10, "x": 0.0, "y": 0.0, "angle": 0.0}
```

### Response Messages (Arduino → ESP32)

#### Status Updates

```json
{
  "position": {
    "x": 25.5,
    "y": 15.2, 
    "angle": 1.57
  },
  "status": "idle",
  "error": 0,
  "pen_down": false
}
```

#### Error Responses

```json
{
  "error": 15,
  "message": "Target position out of bounds", 
  "position": {
    "x": 100.0,
    "y": 100.0,
    "angle": 0.0
  }
}
```

### Status Values

| Status | Description |
|--------|------------ |
| idle | Robot ready for commands |
| moving | Executing movement |
| drawing | Drawing (pen down movement) |
| homing | Returning to origin |
| error | Error state (check error code) |
| emergency_stop | Emergency stop activated |

### Error Codes

| Code | Category | Description |
|------|----------|------------ |
| 0 | None | No error |
| 1-10 | Hardware | Motor, servo, sensor errors |
| 11-20 | Movement | Invalid coordinates, path errors |
| 21-30 | Communication | UART, protocol errors |
| 31-40 | Configuration | Invalid parameters |
| 41-50 | System | Memory, timing errors |

## Implementation Examples

### Desktop GUI Communication

```python
import requests
import json

class RobotController:
    def __init__(self, ip="192.168.4.1", port=80):
        self.base_url = f"http://{ip}:{port}"
    
    def move_to(self, x, y, pen_down=False):
        command = {
            "cmd": 1,
            "x": float(x),
            "y": float(y),
            "pen_down": pen_down
        }
        response = requests.post(f"{self.base_url}/command", 
                               json=command, timeout=5)
        return response.json()
    
    def get_status(self):
        response = requests.get(f"{self.base_url}/status", timeout=5)
        return response.json()
```

### ESP32 UART Bridge

```cpp
void handleCommand(const JsonDocument& cmd) {
    // Forward command to Arduino Nano via UART
    String message;
    serializeJson(cmd, message);
    Serial1.println(message);
    
    // Wait for response
    if (Serial1.available()) {
        String response = Serial1.readStringUntil('\n');
        
        // Parse and validate response
        JsonDocument doc;
        deserializeJson(doc, response);
        
        // Forward to web client
        sendWebResponse(doc);
    }
}
```

### Arduino Nano Command Processing

```cpp
void processCommand() {
    if (Serial.available()) {
        String message = Serial.readStringUntil('\n');
        
        JsonDocument cmd;
        deserializeJson(cmd, message);
        
        int command = cmd["cmd"];
        switch (command) {
            case 1: // Move to coordinate
                moveTo(cmd["x"], cmd["y"]);
                break;
            case 2: // Draw to coordinate  
                drawTo(cmd["x"], cmd["y"]);
                break;
            case 3: // Pen control
                setPen(cmd["pen_down"]);
                break;
        }
        
        // Send status response
        sendStatus();
    }
}
```

## Protocol Validation

Both endpoints validate JSON format and required parameters:

- **Required Fields**: Commands must include `cmd` field
- **Type Validation**: Numeric parameters validated as float/int
- **Range Checking**: Coordinates within valid robot workspace
- **Error Handling**: Invalid commands return structured error responses

## Performance Considerations  

- **Timeout Handling**: 5-second timeout for all HTTP requests
- **Non-blocking UART**: ESP32 uses non-blocking serial to prevent web server delays
- **Message Queuing**: Arduino processes commands sequentially
- **Status Updates**: Automatic status broadcast during movement