#ifndef TERRAPEN_ROBOT_H
#define TERRAPEN_ROBOT_H

#include <Arduino.h>
#include "../hardware/StepperDriver.h"
#include "../hardware/ServoDriver.h"
#include "../TerraPenConfig.h"
#include "../Position.h"

/**
 * Robot state enumeration for state machine
 */
enum RobotState {
    IDLE,           // Ready for commands
    MOVING,         // Executing movement
    ERROR,          // Error state
    EMERGENCY_STOP  // Emergency stop engaged
};

/**
 * TerraPenRobot - Phase 1.5 Implementation
 * 
 * This is the bridge implementation between Phase 1 (hardware drivers) and 
 * Phase 2 (full coordinate system). Provides step-based movement coordination
 * without coordinate mathematics.
 * 
 * Key Phase 1.5 Features:
 * - Coordinates existing non-blocking StepperDriver instances
 * - Basic state machine (IDLE, MOVING, ERROR, EMERGENCY_STOP)
 * - Step-based movement commands (no coordinates yet)
 * - Step counting for future position tracking
 * - Hardware integration for existing drivers
 */
class TerraPenRobot {
private:
    // Hardware drivers (from Phase 1)
    StepperDriver left_motor;
    StepperDriver right_motor;
    ServoDriver pen_servo;
    
    // Robot state
    RobotState state;
    bool pen_is_down;
    
    // Position tracking (Phase 2)
    float current_x;           // Current X position in mm
    float current_y;           // Current Y position in mm  
    float current_angle;       // Current orientation in radians
    
    // Movement coordination state
    int target_left_steps;
    int target_right_steps;
    int current_left_steps;
    int current_right_steps;
    bool movement_active;
    
    // Coordinate movement state (Phase 2)
    float target_x;           // Target X position for coordinate moves
    float target_y;           // Target Y position for coordinate moves
    bool coordinate_movement; // True if executing coordinate-based movement
    float movement_speed_mms; // Movement speed in mm/s
    
    // Step counting for position tracking
    long left_steps_total;
    long right_steps_total;
    
public:
    // === INITIALIZATION ===
    void begin();  // Uses g_config.hardware
    
    // === COORDINATE-BASED MOVEMENT (Phase 2) ===
    bool moveTo(float x, float y, float speed_mms = 15.0);     // Move to coordinates with pen up
    bool drawTo(float x, float y, float speed_mms = 10.0);     // Draw line to coordinates with pen down
    bool moveBy(float dx, float dy, float speed_mms = 15.0);   // Move relative to current position
    bool drawBy(float dx, float dy, float speed_mms = 10.0);   // Draw relative to current position
    
    // === ROTATION CONTROL (Phase 2) ===
    bool turnTo(float angle_radians, float speed_rad_s = 0.5); // Turn to absolute angle
    bool turnBy(float delta_angle, float speed_rad_s = 0.5);   // Turn by relative angle
    
    // === DIRECT HARDWARE CONTROL (Step-based, no coordinates) ===
    bool moveForward(int steps);     // Returns false if busy
    bool moveBackward(int steps);    // Returns false if busy
    bool turnLeft(int steps);        // Returns false if busy
    bool turnRight(int steps);       // Returns false if busy
    
    // === PEN CONTROL ===
    void penUp();
    void penDown();
    bool isPenDown() const;
    
    // === STATE MANAGEMENT ===
    RobotState getState() const;
    bool isBusy() const;             // True if any movement active
    void emergencyStop();
    void clearError();
    
    // === POSITION TRACKING (Phase 2) ===
    Position getCurrentPosition() const;    // Get current position and orientation
    void resetPosition(float x = 0, float y = 0, float angle = 0); // Reset position tracking
    bool isAtTarget() const;                // Check if at target position
    
    // === WORKSPACE SAFETY (Phase 2) ===
    bool isValidPosition(float x, float y) const; // Check workspace boundaries
    
    // === STEP TRACKING (for Phase 2) ===
    long getLeftStepsTotal() const;
    long getRightStepsTotal() const;
    void resetStepCounts();
    
    // === UPDATE FUNCTION ===
    void update();                   // Call every loop iteration - coordinates drivers
    
private:
    // === INTERNAL METHODS ===
    void executeMovement();          // Coordinate stepForward/stepBackward calls
    void setState(RobotState new_state);
    bool isMovementComplete() const;
    void stopAllMotors();
    
    // === KINEMATICS CALCULATIONS (Phase 2) ===
    void calculateSteps(float distance_mm, float angle_diff, int& left_steps, int& right_steps);
    void stepsToMovement(int left_steps, int right_steps, float& distance, float& angle_change);
    void updatePositionEstimate();   // Update position based on step counts
    void executeCoordinateMovement(); // Execute coordinate-based movement
    bool isAtTargetPosition() const;       // Check if at target coordinates
};

#endif // TERRAPEN_ROBOT_H