# Arduino Robot Motion Control Library

This workspace contains an Arduino library for controlling a differential drive wheeled robot with stepper motors and servo pen control.

## Current Status: Phase 1.5 Complete
- Complete robot control class (TerraPenRobot) with state machine
- Step-based movement coordination without coordinate mathematics
- Hardware driver integration and safety features
- Ready for Phase 2 (coordinate system implementation)

## Project Structure
- Arduino library with proper structure (src/, examples/, library.properties)
- Phase 1.5: Complete robot control with step-based movement
- Hardware abstraction for 28BYJ stepper motors via ULN2803A driver
- Servo control for pen up/down mechanism
- State machine with IDLE, MOVING, ERROR, EMERGENCY_STOP states

## Hardware Configuration
- Two 28BYJ stepper motors (motor_l, motor_r) 
- ULN2803A driver IC
- Arduino Nano
- Servo for pen control
- Wheel diameter: 25mm
- Wheelbase: 30mm (15mm from centerline each side)

## Development Guidelines
- Follow Arduino library conventions
- Use TerraPenRobot class for coordinated robot control
- Phase 2 next: Add coordinate system and kinematics
- Provide clear examples and documentation