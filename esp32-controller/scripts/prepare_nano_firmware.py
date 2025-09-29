#!/usr/bin/env python3
"""
PlatformIO build script to prepare Arduino Nano firmware for ESP32 embedding
"""

import os
import shutil
import subprocess
from pathlib import Path

def prepare_nano_firmware(source, target, env):
    """
    Build Nano firmware and prepare it for ESP32 embedding
    """
    print("Building Arduino Nano firmware...")
    
    # Build the Nano firmware in parallel
    nano_project_dir = Path(env.get("PROJECT_DIR")).parent / "nano-firmware"
    if nano_project_dir.exists():
        try:
            # Use PlatformIO to build Nano firmware
            result = subprocess.run([
                "pio", "run", 
                "--project-dir", str(nano_project_dir),
                "--environment", "nano"
            ], capture_output=True, text=True, cwd=str(nano_project_dir))
            
            if result.returncode == 0:
                print("✅ Nano firmware built successfully")
                
                # Copy firmware.hex to ESP32 data directory
                nano_hex = nano_project_dir / ".pio" / "build" / "nano" / "firmware.hex"
                esp32_data = Path(env.get("PROJECT_DIR")) / "data" / "firmware"
                esp32_data.mkdir(parents=True, exist_ok=True)
                
                if nano_hex.exists():
                    shutil.copy2(nano_hex, esp32_data / "nano_firmware.hex")
                    print(f"✅ Copied firmware to {esp32_data / 'nano_firmware.hex'}")
                else:
                    print("❌ Nano firmware.hex not found")
            else:
                print(f"❌ Nano build failed: {result.stderr}")
                
        except Exception as e:
            print(f"❌ Error building Nano firmware: {e}")
    else:
        print(f"❌ Nano project directory not found: {nano_project_dir}")

# Register the build step
if __name__ == "__main__":
    prepare_nano_firmware(None, None, {"PROJECT_DIR": "."})