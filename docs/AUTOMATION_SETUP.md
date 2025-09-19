# TerraPen Motion Control - Automation Setup

## Git Workflow with Testing

### 1. Enable Git Hooks
```powershell
# Make pre-commit hook executable and install
git config core.hooksPath .githooks
```

### 2. Install Dependencies
```powershell
# Install Arduino CLI for automated compilation testing
winget install Arduino.ArduinoCLI

# Configure Arduino CLI
arduino-cli core update-index
arduino-cli core install arduino:avr

# Install Python testing tools (optional)
pip install pytest
```

### 3. Recommended Branch Strategy
```
main branch:     Production-ready code (Phase 1.5 complete)
test branch:     Integration testing with full POST
feature branches: Individual features with unit tests
```

### 4. Automated Test Levels

#### Level 1: Pre-commit (Fast - <30 seconds)
- ✅ Code compilation check
- ✅ Documentation validation  
- ✅ Python simulator tests (if available)

#### Level 2: Pre-push (Medium - <2 minutes)
- ✅ Full unit test compilation
- ✅ Static analysis (if configured)
- ✅ Performance regression check

#### Level 3: CI/CD (Comprehensive - <10 minutes)
- ✅ Multiple Arduino board targets
- ✅ Hardware-in-the-loop tests (if available)
- ✅ Documentation generation

### 5. Usage Examples

```powershell
# Normal development workflow
git add .
git commit -m "Add new feature"  # Triggers pre-commit tests
git push origin feature-branch   # Triggers pre-push tests

# Manual test execution
.\examples\TestRunner\TestRunner.ino  # Upload and run on hardware
python robot_simulator\run_tracker.py # Run simulator tests

# Performance monitoring
# Upload TestRunner, connect serial monitor
# Choose option 3: "Performance Monitoring"
# Let run for 30 seconds to gather baseline data
```

### 6. Test Result Interpretation

#### Quick POST Results:
- **All Pass:** Ready for normal operation
- **1-2 Fail:** Check hardware connections
- **Many Fail:** Power/fundamental issue

#### Full POST Results:
- **Timing Tests Fail:** Clock crystal issue
- **Hardware Tests Fail:** Connection/driver issue  
- **Memory Tests Fail:** RAM/EEPROM problem

#### Performance Monitoring:
- **CPU >80%:** Optimize algorithm timing
- **Memory >90%:** Reduce buffer sizes
- **Step Rate <Expected:** Motor driver issue

### 7. Next Steps for Full Automation

1. **Arduino Cloud Integration:** Upload test results automatically
2. **Hardware-in-Loop:** Remote Arduino testing with real hardware
3. **Regression Testing:** Compare performance against baseline
4. **Release Automation:** Tag versions based on test results

This testing framework provides a solid foundation for Phase 2 development while ensuring Phase 1.5 stability.