/**
 * TerraPen Motion Control Library
 * 
 * Arduino Nano firmware for differential drive robot with pen control
 * Part of the TerraPen Motion System
 * 
 * This is the main include file for the nano firmware.
 * Include this in your Arduino sketches to access all robot functionality.
 */

#pragma once

// Core robot control
#include "src/robot/TerraPenRobot.h"
#include "src/TerraPenConfig.h"

// Hardware drivers
#include "src/hardware/StepperDriver.h"
#include "src/hardware/ServoDriver.h"

// System components
#include "src/ErrorSystem.h"
#include "src/PerformanceMonitor.h"

// Storage and communication
#include "src/storage/NVRAMManager.h"

// Position and math utilities
#include "src/Position.h"

// Main library version
#define TERRAPEN_MOTION_VERSION "2.0.0"
#define TERRAPEN_MOTION_VERSION_MAJOR 2
#define TERRAPEN_MOTION_VERSION_MINOR 0
#define TERRAPEN_MOTION_VERSION_PATCH 0

/**
 * Quick start: Create a robot instance
 * 
 * TerraPenRobot robot;
 * robot.begin();
 * robot.moveTo(100, 100);  // Move to coordinate (100mm, 100mm)
 */