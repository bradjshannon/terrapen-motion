# Arduino Robot Motion Control Library

This workspace contains an Arduino library for controlling a differential drive wheeled robot with stepper motors and servo pen control.

## Project Structure
- Arduino library with proper structure (src/, examples/, library.properties)
- Motion control API for path segments with configurable parameters
- Differential drive kinematics implementation
- Hardware abstraction for 28BYJ stepper motors via ULN2803A driver
- Servo control for pen up/down mechanism

## Hardware Configuration
- Two 28BYJ stepper motors (motor_l, motor_r) 
- ULN2803A driver IC
- Arduino Nano
- Servo for pen control
- Wheel diameter: 25mm
- Wheelbase: 30mm (15mm from centerline each side)

## Development Guidelines
- Follow Arduino library conventions
- Use configurable parameters for hardware specifications
- Implement path segments with motor direction/magnitude/speed control
- Provide clear examples and documentation