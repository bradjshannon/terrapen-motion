#ifndef ROBOT_CONFIG_H
#define ROBOT_CONFIG_H

#include <Arduino.h>

/**
 * RobotConfig - Hardware configuration and parameters for TerraPen robot
 * 
 * Contains all hardware-specific settings including:
 * - Physical robot dimensions
 * - Pin assignments for motors and servo
 * - Performance limits and safety constraints
 * - Default operational parameters
 * 
 * Usage:
 *   RobotConfig config;           // Use defaults
 *   config.wheel_diameter_mm = 30; // Override as needed
 *   robot.begin(config);
 */
struct RobotConfig {
    // === PHYSICAL PARAMETERS ===
    
    /** Wheel diameter in millimeters */
    float wheel_diameter_mm;
    
    /** Distance between wheel centers in millimeters */
    float wheelbase_mm;
    
    /** Steps per full revolution for stepper motors (28BYJ-48: 2048) */
    int steps_per_revolution;
    
    // === PIN ASSIGNMENTS ===
    
    /** Left motor pin assignments [IN1, IN2, IN3, IN4] */
    int left_motor_pins[4];
    
    /** Right motor pin assignments [IN1, IN2, IN3, IN4] */
    int right_motor_pins[4];
    
    /** Servo motor pin for pen control */
    int servo_pin;
    
    // === SERVO CONFIGURATION ===
    
    /** Servo angle for pen up position (degrees) */
    int pen_up_angle;
    
    /** Servo angle for pen down position (degrees) */
    int pen_down_angle;
    
    // === PERFORMANCE LIMITS ===
    
    /** Maximum linear speed in mm/s */
    float max_speed_mms;
    
    /** Maximum angular speed in rad/s */
    float max_angular_speed_rad_s;
    
    /** Default movement speed in mm/s */
    float default_speed_mms;
    
    /** Default drawing speed in mm/s (typically slower) */
    float default_draw_speed_mms;
    
    // === WORKSPACE LIMITS ===
    
    /** Minimum X coordinate (mm) */
    float min_x;
    
    /** Maximum X coordinate (mm) */
    float max_x;
    
    /** Minimum Y coordinate (mm) */
    float min_y;
    
    /** Maximum Y coordinate (mm) */
    float max_y;
    
    // === CONSTRUCTOR WITH DEFAULTS ===
    
    /**
     * Default constructor with typical TerraPen configuration
     * Based on common 28BYJ-48 stepper setup with Arduino Nano
     */
    RobotConfig() :
        // Physical parameters (typical for small drawing robot)
        wheel_diameter_mm(25.0),
        wheelbase_mm(30.0),
        steps_per_revolution(2048),  // 28BYJ-48 with gear reduction
        
        // Servo configuration
        servo_pin(9),
        pen_up_angle(90),
        pen_down_angle(0),
        
        // Performance limits
        max_speed_mms(50.0),
        max_angular_speed_rad_s(1.0),
        default_speed_mms(15.0),
        default_draw_speed_mms(10.0),
        
        // Workspace limits (adjust for your robot)
        min_x(-100.0), max_x(100.0),
        min_y(-100.0), max_y(100.0)
    {
        // Default pin assignments for Arduino Nano
        // Left motor (motor A)
        left_motor_pins[0] = 2;   // IN1
        left_motor_pins[1] = 3;   // IN2
        left_motor_pins[2] = 4;   // IN3
        left_motor_pins[3] = 5;   // IN4
        
        // Right motor (motor B)
        right_motor_pins[0] = 6;  // IN1
        right_motor_pins[1] = 7;  // IN2
        right_motor_pins[2] = 8;  // IN3
        right_motor_pins[3] = 10; // IN4 (pin 9 used for servo)
    }
    
    // === VALIDATION METHODS ===
    
    /**
     * Check if configuration values are valid
     * @return true if all parameters are within reasonable ranges
     */
    bool isValid() const {
        return (wheel_diameter_mm > 0 && wheel_diameter_mm < 200) &&
               (wheelbase_mm > 0 && wheelbase_mm < 500) &&
               (steps_per_revolution > 0 && steps_per_revolution < 10000) &&
               (servo_pin >= 0 && servo_pin <= 13) &&
               (pen_up_angle >= 0 && pen_up_angle <= 180) &&
               (pen_down_angle >= 0 && pen_down_angle <= 180) &&
               (max_speed_mms > 0 && max_speed_mms < 1000) &&
               (max_angular_speed_rad_s > 0 && max_angular_speed_rad_s < 10) &&
               (max_x > min_x) && (max_y > min_y);
    }
    
    /**
     * Get steps per millimeter based on wheel diameter
     * @return Steps needed to travel 1mm
     */
    float getStepsPerMM() const {
        float circumference = PI * wheel_diameter_mm;
        return steps_per_revolution / circumference;
    }
    
    /**
     * Get millimeters per step
     * @return Distance traveled per step in mm
     */
    float getMMPerStep() const {
        float circumference = PI * wheel_diameter_mm;
        return circumference / steps_per_revolution;
    }
    
    /**
     * Validate workspace coordinates
     * @param x X coordinate to check
     * @param y Y coordinate to check
     * @return true if coordinates are within workspace limits
     */
    bool isInWorkspace(float x, float y) const {
        return (x >= min_x && x <= max_x && y >= min_y && y <= max_y);
    }
    
    /**
     * Print configuration to Serial for debugging
     */
    void printConfig() const {
        Serial.println("=== Robot Configuration ===");
        Serial.print("Wheel diameter: "); Serial.print(wheel_diameter_mm); Serial.println(" mm");
        Serial.print("Wheelbase: "); Serial.print(wheelbase_mm); Serial.println(" mm");
        Serial.print("Steps/rev: "); Serial.println(steps_per_revolution);
        Serial.print("Steps/mm: "); Serial.println(getStepsPerMM());
        Serial.print("Servo pin: "); Serial.println(servo_pin);
        Serial.print("Pen angles: "); Serial.print(pen_up_angle); Serial.print("° up, "); 
        Serial.print(pen_down_angle); Serial.println("° down");
        Serial.print("Max speed: "); Serial.print(max_speed_mms); Serial.println(" mm/s");
        Serial.print("Workspace: ("); Serial.print(min_x); Serial.print(","); Serial.print(min_y);
        Serial.print(") to ("); Serial.print(max_x); Serial.print(","); Serial.print(max_y); Serial.println(")");
        
        Serial.print("Left motor pins: ");
        for (int i = 0; i < 4; i++) {
            Serial.print(left_motor_pins[i]);
            if (i < 3) Serial.print(", ");
        }
        Serial.println();
        
        Serial.print("Right motor pins: ");
        for (int i = 0; i < 4; i++) {
            Serial.print(right_motor_pins[i]);
            if (i < 3) Serial.print(", ");
        }
        Serial.println();
        Serial.println("===========================");
    }
};

#endif // ROBOT_CONFIG_H