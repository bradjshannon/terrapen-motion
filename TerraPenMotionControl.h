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
 * Version: 2.0.0 (Phase 2 Complete)
 * Author: TerraPen Project
 * 
 * Hardware Requirements:
 * - Arduino Nano or compatible
 * - Two 28BYJ-48 stepper motors
 * - ULN2803A or similar driver IC
 * - Servo motor for pen control
 * - 5V power supply
 * 
 * Basic Usage (Phase 2 - Coordinate-based movement):
 * 
 *   #include <TerraPenMotionControl.h>
 *   
 *   TerraPenRobot robot;
 *   
 *   void setup() {
 *     robot.begin();
 *     robot.resetPosition(0, 0, 0);  // Set coordinate origin
 *   }
 *   
 *   void loop() {
 *     robot.update();
 *     
 *     if (!robot.isBusy()) {
 *       robot.moveTo(50, 50);    // Move to coordinates (pen up)
 *       robot.drawTo(100, 50);   // Draw to coordinates (pen down)
 *     }
 *   }
 * 
 * Alternative Usage (Phase 1.5 - Step-based robot control):
 * 
 *   #include <TerraPenMotionControl.h>
 *   
 *   TerraPenRobot robot;
 *   
 *   void setup() {
 *     robot.begin();
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
#include "src/TerraPenConfig.h"
#include "src/Position.h"

// Hardware drivers
#include "src/hardware/StepperDriver.h"
#include "src/hardware/ServoDriver.h"

// Robot control (Phase 2 complete)
#include "src/robot/TerraPenRobot.h"

// System components (optional - for advanced usage)
#include "src/ErrorSystem.h"
#include "src/PerformanceMonitor.h"
#include "src/storage/NVRAMManager.h"
#include "src/communication/ESP32Uploader.h"
#include "src/testing/TestFramework.h"
#include "src/testing/PowerOnSelfTest.h"

// Version information
#define TERRAPEN_MOTION_CONTROL_VERSION "2.0.0"
#define TERRAPEN_MOTION_CONTROL_VERSION_MAJOR 2
#define TERRAPEN_MOTION_CONTROL_VERSION_MINOR 0
#define TERRAPEN_MOTION_CONTROL_VERSION_PATCH 0

#endif // TERRAPEN_MOTION_CONTROL_H