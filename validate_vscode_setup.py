#!/usr/bin/env python3
"""
VSCode Test Integration Validator

This script validates that the VSCode testing integration is properly configured.
Run this to verify all components are set up correctly.
"""

import os
import json
import sys
from pathlib import Path

def check_file_exists(filepath, description):
    """Check if a file exists and print status."""
    if os.path.exists(filepath):
        print(f"✅ {description}: {filepath}")
        return True
    else:
        print(f"❌ {description}: {filepath} (MISSING)")
        return False

def validate_json_file(filepath, required_keys, description):
    """Validate JSON file structure."""
    if not os.path.exists(filepath):
        print(f"❌ {description}: {filepath} (FILE MISSING)")
        return False
    
    try:
        with open(filepath, 'r') as f:
            content = f.read()
        
        # Handle JSONC (JSON with comments) for VSCode files
        if filepath.endswith('.json') and '//' in content:
            # Remove single-line comments for parsing
            lines = content.split('\n')
            cleaned_lines = []
            for line in lines:
                if line.strip().startswith('//'):
                    continue  # Skip comment lines
                elif '//' in line:
                    # Remove inline comments
                    line = line.split('//')[0].rstrip()
                cleaned_lines.append(line)
            content = '\n'.join(cleaned_lines)
        
        data = json.loads(content)
        
        missing_keys = []
        for key in required_keys:
            if key not in data:
                missing_keys.append(key)
        
        if missing_keys:
            print(f"❌ {description}: Missing keys: {missing_keys}")
            return False
        else:
            print(f"✅ {description}: All required keys present")
            return True
            
    except json.JSONDecodeError as e:
        print(f"❌ {description}: Invalid JSON - {e}")
        return False
    except Exception as e:
        print(f"❌ {description}: Error - {e}")
        return False

def main():
    """Main validation function."""
    print("TerraPen VSCode Test Integration Validator")
    print("=" * 50)
    
    # Check if we're in the right directory
    if not os.path.exists("platformio.ini"):
        print("❌ Error: Not in TerraPen project directory (missing platformio.ini)")
        sys.exit(1)
    
    all_good = True
    
    # VSCode configuration files
    vscode_files = [
        (".vscode/settings.json", "VSCode Settings"),
        (".vscode/tasks.json", "VSCode Tasks"),
        (".vscode/launch.json", "VSCode Debug Configuration"),
        (".vscode/keybindings.json", "VSCode Key Bindings"),
        (".vscode/problem-matchers.json", "VSCode Problem Matchers"),
        (".vscode/README.md", "VSCode Documentation")
    ]
    
    print("\n📁 VSCode Configuration Files:")
    for filepath, desc in vscode_files:
        if not check_file_exists(filepath, desc):
            all_good = False
    
    # Test directory structure
    test_dirs = [
        ("test/framework", "Test Framework Directory"),
        ("test/unit", "Unit Tests Directory"),
        ("test/hardware", "Hardware Tests Directory"),
        ("test/integration", "Integration Tests Directory")
    ]
    
    print("\n📁 Test Directory Structure:")
    for dirpath, desc in test_dirs:
        if not check_file_exists(dirpath, desc):
            all_good = False
    
    # Key test framework files
    framework_files = [
        ("test/framework/TestFramework.h", "Test Framework Header"),
        ("test/framework/TestFramework.cpp", "Test Framework Implementation"),
        ("test/framework/PowerOnSelfTest.h", "POST System Header"),
        ("test/README.md", "Test Documentation")
    ]
    
    print("\n📄 Test Framework Files:")
    for filepath, desc in framework_files:
        if not check_file_exists(filepath, desc):
            all_good = False
    
    # Validate JSON configurations
    print("\n🔧 Configuration Validation:")
    
    # Tasks.json validation
    tasks_required = ["version", "tasks"]
    if not validate_json_file(".vscode/tasks.json", tasks_required, "Tasks Configuration"):
        all_good = False
    
    # Settings.json validation  
    settings_required = ["C_Cpp.default.includePath", "files.associations"]
    if not validate_json_file(".vscode/settings.json", settings_required, "Settings Configuration"):
        all_good = False
    
    # Launch.json validation
    launch_required = ["version", "configurations"]
    if not validate_json_file(".vscode/launch.json", launch_required, "Debug Configuration"):
        all_good = False
    
    # Check PlatformIO test environments
    print("\n⚙️  PlatformIO Test Environments:")
    try:
        with open("platformio.ini", 'r') as f:
            content = f.read()
        
        required_envs = ["test-logic", "test-hardware", "test-all", "test-post"]
        for env in required_envs:
            if f"[env:{env}]" in content:
                print(f"✅ Test Environment: {env}")
            else:
                print(f"❌ Test Environment: {env} (MISSING)")
                all_good = False
                
    except Exception as e:
        print(f"❌ PlatformIO Config: Error reading platformio.ini - {e}")
        all_good = False
    
    # Summary
    print("\n" + "=" * 50)
    if all_good:
        print("🎉 VSCode Test Integration: FULLY CONFIGURED")
        print("\nQuick Start:")
        print("  Ctrl+Shift+T - Run all tests")
        print("  Ctrl+Alt+T   - Run unit tests")
        print("  Ctrl+Shift+H - Run hardware tests")
        print("\nOpen VSCode and start testing!")
    else:
        print("❌ VSCode Test Integration: ISSUES FOUND")
        print("\nPlease fix the missing files/configurations above.")
        sys.exit(1)

if __name__ == "__main__":
    main()