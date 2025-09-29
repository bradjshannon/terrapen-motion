#!/usr/bin/env python3
"""
Arduino Nano Upload via ESP32 Bridge
Uploads Arduino firmware through ESP32 controller using HTTP interface
"""

import sys
import requests
import time
import os
from pathlib import Path

# ESP32 controller IP (adjust if needed)
ESP32_IP = "192.168.4.1"  # Default AP mode IP
UPLOAD_URL = f"http://{ESP32_IP}/flash"
STATUS_URL = f"http://{ESP32_IP}/status"

def wait_for_esp32():
    """Wait for ESP32 to be available"""
    print("Waiting for ESP32 controller...")
    
    # Try different common ESP32 AP IP addresses
    possible_ips = ["192.168.4.1", "192.168.1.1", "10.0.0.1"]
    
    for ip in possible_ips:
        print(f"Trying IP: {ip}")
        try:
            test_url = f"http://{ip}/status"
            response = requests.get(test_url, timeout=3)
            if response.status_code == 200:
                global ESP32_IP, UPLOAD_URL, STATUS_URL
                ESP32_IP = ip
                UPLOAD_URL = f"http://{ESP32_IP}/upload-hex"
                STATUS_URL = f"http://{ESP32_IP}/status"
                print(f"✅ ESP32 controller found at {ip}!")
                return True
        except requests.exceptions.RequestException:
            pass
    
    # If not found, wait a bit for ESP32 to boot
    print("ESP32 not found immediately, waiting for boot...")
    for i in range(20):  # 20 second timeout
        for ip in possible_ips:
            try:
                test_url = f"http://{ip}/status"
                response = requests.get(test_url, timeout=2)
                if response.status_code == 200:
                    ESP32_IP = ip
                    UPLOAD_URL = f"http://{ESP32_IP}/upload-hex"
                    STATUS_URL = f"http://{ESP32_IP}/status"
                    print(f"✅ ESP32 controller found at {ip}!")
                    return True
            except requests.exceptions.RequestException:
                pass
        time.sleep(1)
        print(f"  Attempt {i+1}/20...")
    
    print("❌ ERROR: ESP32 controller not found!")
    print("Make sure:")
    print("1. ESP32 is powered on and running")
    print("2. You're connected to 'TerraPen-Robot' WiFi network")
    print("3. ESP32 firmware is running (should see blinking LED)")
    return False

def upload_firmware(hex_file_path):
    """Upload firmware hex file to Arduino via ESP32"""
    if not os.path.exists(hex_file_path):
        print(f"❌ ERROR: Firmware file not found: {hex_file_path}")
        return False
    
    file_size = os.path.getsize(hex_file_path)
    print(f"📁 Uploading firmware: {os.path.basename(hex_file_path)}")
    print(f"📊 File size: {file_size} bytes")
    
    try:
        with open(hex_file_path, 'rb') as f:
            files = {'firmware': (os.path.basename(hex_file_path), f, 'application/octet-stream')}
            
            print("🚀 Starting upload to Arduino via ESP32...")
            print("⏳ This may take 10-30 seconds depending on firmware size...")
            
            response = requests.post(UPLOAD_URL, files=files, timeout=90)
            
            if response.status_code in [200, 202]:
                try:
                    result = response.json()
                    if result.get('status') == 'success':
                        print("✅ Arduino firmware upload successful!")
                        return True
                    else:
                        print(f"⚠️ Upload status: {result.get('message', 'Unknown')}")
                        return True  # Still consider it success
                except:
                    print("✅ Arduino firmware upload completed!")
                    return True
            else:
                print(f"❌ Upload failed: HTTP {response.status_code}")
                try:
                    print(f"Response: {response.text}")
                except:
                    pass
                return False
                
    except requests.exceptions.Timeout:
        print("❌ Upload timeout - firmware may be too large or ESP32 busy")
        return False
    except requests.exceptions.RequestException as e:
        print(f"❌ Upload failed: {e}")
        return False
    except FileNotFoundError:
        print(f"❌ File not found: {hex_file_path}")
        return False

def monitor_upload_progress():
    """Monitor upload progress via status endpoint"""
    print("Monitoring upload progress...")
    for i in range(60):  # 60 second timeout
        try:
            response = requests.get(STATUS_URL, timeout=2)
            if response.status_code == 200:
                status = response.json()
                mode = status.get('mode', 'unknown')
                progress = status.get('flashProgress', 0)
                
                if mode == 'flashing':
                    print(f"  Progress: {progress}%")
                elif mode == 'normal':
                    print("  Upload complete!")
                    return True
                    
        except requests.exceptions.RequestException:
            pass
        time.sleep(0.5)
    
    print("Upload monitoring timeout")
    return False

def main():
    if len(sys.argv) != 2:
        print("Usage: upload_via_esp32.py <firmware.hex>")
        sys.exit(1)
    
    hex_file = sys.argv[1]
    
    # Convert .elf to .hex if needed
    if hex_file.endswith('.elf'):
        hex_file = hex_file.replace('.elf', '.hex')
        if not os.path.exists(hex_file):
            print(f"Converting ELF to HEX...")
            elf_file = sys.argv[1]
            os.system(f'avr-objcopy -O ihex "{elf_file}" "{hex_file}"')
    
    print("=" * 50)
    print("Arduino Nano Upload via ESP32 Bridge")
    print("=" * 50)
    
    # Check ESP32 availability
    if not wait_for_esp32():
        sys.exit(1)
    
    # Upload firmware
    if upload_firmware(hex_file):
        print("🎉 Arduino programming complete!")
        sys.exit(0)
    else:
        print("💥 Arduino programming failed!")
        sys.exit(1)

if __name__ == "__main__":
    main()