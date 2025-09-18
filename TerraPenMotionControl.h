#ifndef TERRAPEN_MOTION_CONTROL_H
#define TERRAPEN_MOTION_CONTROL_H

/**
 * TerraPen Motion Control Library
 * 
 * Arduino library for controlling differential drive wheeled robots 
 * with stepper motors and servo pen control.
 * 
 * This library provides:
 * - Hardware drivers for 28BYJ-48 stepper motors and servo control
 * - Robot configuration management
 * - Position tracking and coordinate system utilities
 * - Complete robot control class (Phase 1.5+)
 * - Support for drawing robot applications
 * 
 * Version: 1.1.0 (Phase 1.5)
 * Author: TerraPen Project
 * 
 * Hardware Requirements:
 * - Arduino Nano or compatible
 * - Two 28BYJ-48 stepper motors
 * - ULN2803A or similar driver IC
 * - Servo motor for pen control
 * - 5V power supply
 * 
 * Basic Usage (Phase 1.5 - Step-based robot control):
 * 
 *   #include <TerraPenMotionControl.h>
 *   
 *   TerraPenRobot robot;
 *   RobotConfig config;
 *   
 *   void setup() {
 *     robot.begin(config);
 *   }
 *   
 *   void loop() {
 *     robot.update();
 *     
 *     if (!robot.isBusy()) {
 *       robot.moveForward(50);  // Move 50 steps forward
 *     }
 *   }
 * 
 * Advanced Usage (Phase 1 - Direct hardware control):
 * 
 *   StepperDriver left_motor, right_motor;
 *   ServoDriver pen;
 *   
 *   void setup() {
 *     left_motor.begin(2, 3, 4, 5);
 *     right_motor.begin(6, 7, 8, 9);
 *     pen.begin(10);
 *   }
 *   
 *   void loop() {
 *     left_motor.stepForward();
 *     right_motor.stepForward();
 *     pen.update();
 *   }
 */

// Core data structures
#include "src/RobotConfig.h"
#include "src/Position.h"

// Hardware drivers
#include "src/hardware/StepperDriver.h"
#include "src/hardware/ServoDriver.h"

// Robot control (Phase 1.5+)
#include "src/robot/TerraPenRobot.h"

// Version information
#define TERRAPEN_MOTION_CONTROL_VERSION "1.1.0"
#define TERRAPEN_MOTION_CONTROL_VERSION_MAJOR 1
#define TERRAPEN_MOTION_CONTROL_VERSION_MINOR 1
#define TERRAPEN_MOTION_CONTROL_VERSION_PATCH 0

#endif // TERRAPEN_MOTION_CONTROL_H