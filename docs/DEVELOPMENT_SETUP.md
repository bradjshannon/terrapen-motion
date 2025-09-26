# Development Environment Setup Guide

## Overview

This guide provides comprehensive setup instructions for the TerraPen Motion Control Library development environment.

## Quick Start

### Prerequisites
- **VSCode**: Latest version with C++ and Python extensions
- **Python**: 3.11 or newer
- **PlatformIO Core**: For Arduino development and testing
- **Git**: For version control

### Initial Setup
1. **Clone Repository**:
   ```bash
   git clone https://github.com/bradjshannon/terrapen-motion.git
   cd terrapen-motion
   ```

2. **Open in VSCode**:
   ```bash
   code .
   ```

3. **Install Recommended Extensions**:
   VSCode will prompt to install recommended extensions automatically.

4. **Activate Python Environment**:
   The virtual environment will be automatically configured when you open the project.

## Development Environment Features

### Testing
- **Mathematical validation** with comprehensive coordinate system tests
- **Simple execution** via command line or batch files
- **Zero-Hardware Testing**: Core algorithms validated without Arduino
- **Test Categories**: Distance, Angle, Position, Interpolation, Precision
- **Simple Testing**: Run comprehensive algorithm validation

### PlatformIO Integration
- **Multiple Build Environments**:
  - `nano`: Arduino Nano production build
  - `test-math`: Mathematical validation (no hardware)
  - `test-integration`: Hardware integration testing
- **Library Dependencies**: Automatically managed
- **Upload and Monitor**: Integrated serial communication

### Python Robot Simulator
- **Location**: `robot_simulator/` directory
- **Features**: Real-time robot motion visualization
- **GUI Interface**: Track robot movement and pen operations
- **Standalone Operation**: Test algorithms without hardware

## Testing Workflow

### Mathematical Algorithm Validation
```powershell
# Quick test via command line
pio run -e test-math

# Or via command line
.venv\Scripts\Activate.ps1
python .vscode\test_discovery.py "." run-suite math-validation
```

### Hardware Integration Testing
```powershell
# Connect Arduino Nano with motors and servo
pio test -e test-integration
```

### Build Validation
```powershell
# Compile without upload
pio run -e nano

# Check program size
pio run -e nano --target size
```

## Project Structure

```
terrapen-motion/
├── .vscode/                    # VSCode configuration and testing
│   ├── test_discovery.py      # Test discovery system
│   ├── test-runner-config.json # Test definitions
│   ├── settings.json          # Editor configuration
│   └── tasks.json             # Build and test tasks
├── .venv/                     # Python virtual environment
├── docs/                      # Project documentation
│   ├── API_REFERENCE.md       # Class and method documentation
│   ├── ARCHITECTURE.md        # System architecture
│   ├── VSCODE_TESTING_INTEGRATION.md # Testing setup guide
│   └── TESTING.md            # Testing methodology
├── examples/                  # Arduino example sketches
├── robot_simulator/           # Python robot simulator
├── src/                      # Library source code
│   ├── robot/                # TerraPenRobot class
│   ├── hardware/             # Motor and servo drivers
│   ├── storage/              # NVRAM management
│   └── Position.h            # Core coordinate mathematics
├── test/                     # Test files
├── library.properties        # Arduino library metadata
├── platformio.ini           # PlatformIO configuration
└── requirements.txt         # Python dependencies
```

## Common Development Tasks

### Adding New Features
1. **Create Feature Branch**:
   ```bash
   git checkout -b feature/new-functionality
   ```

2. **Implement Code**: Add to appropriate `src/` directory

3. **Write Tests**: Add test cases to test configuration

4. **Validate**: Run mathematical tests to ensure no regressions
   ```powershell
   python .vscode\test_discovery.py "." run-suite math-validation
   ```

5. **Test Hardware** (if applicable):
   ```powershell
   pio test -e test-integration
   ```

### Debugging
- **VSCode Debugging**: Use F5 to debug Arduino code
- **Serial Monitor**: Built into VSCode for hardware communication
- **Test Debugging**: Step through mathematical algorithms

### Documentation Updates
- **API Changes**: Update `docs/API_REFERENCE.md`
- **Architecture Changes**: Update `docs/ARCHITECTURE.md`
- **New Examples**: Add to `examples/` with documentation

## Advanced Features

### Performance Monitoring
The library includes built-in performance monitoring:
- Memory usage tracking
- Execution time measurement
- Error rate monitoring
- EEPROM wear leveling

### Error Handling System
Comprehensive error handling with:
- Error classification and logging
- Recovery mechanisms
- Diagnostic information
- Hardware fault detection

### Configuration Management
- **Compile-time Configuration**: `TerraPenConfig.h`
- **Runtime Parameters**: NVRAM storage
- **Example Configurations**: Multiple hardware setups

## Troubleshooting

### Common Issues

**Tests Not Running**
- Ensure PlatformIO Core is installed and updated
- Run "pio run -e test-math" to verify build
- Check that project builds successfully

**PlatformIO Build Errors**
- Update PlatformIO Core: `pio upgrade`
- Clean build: `pio run --target clean`
- Check library dependencies in `platformio.ini`

**Python Environment Issues**
- Recreate virtual environment: `python -m venv .venv`
- Install dependencies: `pip install -r requirements.txt`
- Check Python version: `python --version` (should be 3.11+)

**Hardware Connection Problems**
- Check Arduino drivers
- Verify serial port in Device Manager
- Test with basic Arduino sketch first

### Getting Help
- **Documentation**: Comprehensive guides in `docs/` folder
- **Examples**: Working code in `examples/` directory  
- **Issues**: Report problems via GitHub Issues
- **Discussions**: Ask questions via GitHub Discussions

## Version Information

### Current Phase: Phase 2 Complete + VSCode Integration
- ✅ **Core Robot Control**: Complete TerraPenRobot class
- ✅ **Coordinate System**: Full 2D coordinate mathematics
- ✅ **Hardware Abstraction**: Driver classes for motors and servos
- ✅ **Testing Framework**: 23 mathematical validation tests
- ✅ **Simple Testing**: Mathematical validation without complexity
- ✅ **Development Tools**: Python simulator and debugging

### Upcoming Features
- Enhanced path planning algorithms
- Advanced error recovery mechanisms
- Extended hardware configuration options
- Performance optimization tools

The TerraPen Motion Control Library provides a complete, professional development environment for creating precision drawing robots with comprehensive testing and validation capabilities.