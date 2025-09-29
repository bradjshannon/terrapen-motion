# TerraPen Dual-Device Development Workflow

This document describes how to seamlessly develop and deploy firmware to both the ESP32 controller and Arduino Nano using VS Code and PlatformIO.

## 🎯 **Quick Start - One-Click Workflows**

### **VS Code Tasks (Recommended)**

Open the Command Palette (`Ctrl+Shift+P`) and type "Tasks: Run Task", then select:

1. **`Build All`** - Build both ESP32 and Arduino firmware in parallel
2. **`Upload ESP32`** - Upload ESP32 via USB  
3. **`Upload ESP32 (OTA)`** - Upload ESP32 over WiFi
4. **`Upload Arduino (via ESP32)`** - Upload Arduino firmware through ESP32 bridge
5. **`Build & Upload Both`** - Complete deployment sequence
6. **`Flash Development Setup`** - Flash ESP32 via USB, then Arduino via ESP32

### **PlatformIO Project Picker**

The PlatformIO extension will show both projects in the project picker:
- `nano-firmware` - Arduino Nano development
- `esp32-controller` - ESP32 development

Switch between projects to use the normal PlatformIO build/upload buttons.

## 🔧 **Workflow Options**

### **Option 1: Arduino-like Experience (Simplest)**

**For ESP32:**
```bash
cd esp32-controller
pio run -t upload                    # Via USB
pio run -t upload --upload-port terrapen-esp32.local  # Via OTA
```

**For Arduino via ESP32 Bridge:**
```bash
cd nano-firmware  
pio run                              # Build Arduino firmware
cd ../esp32-controller
powershell -File scripts/upload_via_esp32.ps1 ../nano-firmware/.pio/build/nano/firmware.hex
```

### **Option 2: VS Code Tasks (Recommended)**

Use the tasks defined in `.vscode/tasks.json`:
- All builds and uploads are automated
- Parallel building for faster development
- Integrated error reporting
- One-click deployment sequences

### **Option 3: Direct PlatformIO Environments**

Each project has its own `platformio.ini` with multiple environments:

**ESP32 Controller:**
- `esp32-s3-zero` - Main ESP32 firmware

**Arduino Nano:**
- `nano` - Main Arduino firmware
- `test-math` - Mathematical validation tests
- `test-integration` - Hardware integration tests

## 📡 **Arduino Upload Methods**

### **Method 1: ESP32 Bridge (Wireless)**
- ESP32 acts as wireless programmer for Arduino
- No USB cables needed for Arduino
- Works through WiFi web interface or HTTP API
- Requires ESP32 to be running and accessible

### **Method 2: Direct USB (Traditional)**
- Direct USB connection to Arduino
- Standard PlatformIO upload
- Use when ESP32 bridge is not available

## 🌐 **ESP32 Bridge Setup**

### **WiFi Network:**
- **SSID:** `TerraPen-Robot`
- **Password:** `terrapen123`
- **IP:** `192.168.4.1` (usually)

### **Web Interface:**
1. Connect to WiFi network
2. Open browser to `http://192.168.4.1`
3. Upload `.hex` file through web form
4. Monitor progress in real-time

### **HTTP API:**
```bash
# Upload firmware
curl -X POST -F "firmware=@firmware.hex" http://192.168.4.1/upload-hex

# Check status  
curl http://192.168.4.1/status

# Reset Arduino
curl -X POST http://192.168.4.1/reset
```

## 🔄 **Development Workflow Examples**

### **Typical Development Session:**

1. **Start Development:**
   ```
   Task: "Flash Development Setup"
   → Uploads ESP32 via USB
   → Uploads Arduino via ESP32 bridge
   ```

2. **Iterate on Code:**
   ```
   Task: "Build All" (while coding)
   Task: "Upload Arduino (via ESP32)" (test changes)
   ```

3. **Deploy Updates:**
   ```
   Task: "Upload ESP32 (OTA)" (if ESP32 changed)
   Task: "Upload Arduino (via ESP32)" (if Arduino changed)
   ```

### **Quick Arduino-Only Updates:**
```bash
# From VS Code terminal:
cd nano-firmware
pio run
cd ../esp32-controller  
powershell scripts/upload_via_esp32.ps1 ../nano-firmware/.pio/build/nano/firmware.hex
```

### **Complete System Update:**
```
VS Code Task: "Build & Upload Both"
```

## 🛠 **Troubleshooting**

### **ESP32 Bridge Not Found:**
1. Check ESP32 is powered and running (LED should blink)
2. Verify WiFi connection to `TerraPen-Robot`
3. Try different IP addresses: `192.168.4.1`, `192.168.1.1`, `10.0.0.1`
4. Check ESP32 serial output for actual IP

### **Arduino Upload Fails:**
1. Verify ESP32 bridge is accessible
2. Check `.hex` file exists: `nano-firmware/.pio/build/nano/firmware.hex`
3. Try direct USB upload: `cd nano-firmware && pio run -t upload`
4. Reset Arduino manually via ESP32 web interface

### **Build Errors:**
1. Clean and rebuild: `pio run -t clean && pio run`
2. Check PlatformIO project selection
3. Verify all dependencies are installed
4. Check serial port permissions (Linux/Mac)

## 📁 **File Structure**

```
terrapen-motion/
├── .vscode/
│   ├── tasks.json           # VS Code build tasks
│   └── settings.json        # Multi-project settings
├── esp32-controller/
│   ├── platformio.ini       # ESP32 build config
│   ├── src/main.cpp         # ESP32 firmware
│   └── scripts/
│       ├── upload_via_esp32.py   # Python upload script
│       └── upload_via_esp32.ps1  # PowerShell upload script
├── nano-firmware/
│   ├── platformio.ini       # Arduino build config
│   └── src/                 # Arduino source code
└── platformio.ini           # Workspace reference (optional)
```

## 🎮 **VS Code Extensions Required**

- **PlatformIO IDE** - Main development environment
- **C/C++** - Code intelligence and debugging
- **Task Runner** (optional) - Enhanced task management

## 💡 **Pro Tips**

1. **Use OTA for ESP32** - Much faster than USB after initial flash
2. **Keep ESP32 Bridge Running** - Leave ESP32 powered for quick Arduino updates
3. **Parallel Development** - Use "Build All" task to build both devices while coding
4. **Serial Monitoring** - Use PlatformIO serial monitor for debugging
5. **Task Sequences** - Combine tasks for one-click deployment workflows

## 🔗 **Integration with Your Workflow**

This setup treats the Arduino almost like another ESP32 environment in PlatformIO. You get:

- ✅ **One-click uploads** via VS Code tasks
- ✅ **No manual web interface** needed
- ✅ **Parallel building** for both devices
- ✅ **Integrated error reporting**
- ✅ **Automated deployment sequences**
- ✅ **Same workflow as ESP32** OTA updates

The ESP32 becomes a "wireless programmer" for the Arduino, giving you the convenience of network-based firmware updates for both devices!