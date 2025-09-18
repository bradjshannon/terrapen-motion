#ifndef TERRAPEN_ROBOT_H
#define TERRAPEN_ROBOT_H

#include <Arduino.h>
#include "../hardware/StepperDriver.h"
#include "../hardware/ServoDriver.h"
#include "../RobotConfig.h"

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
    
    // Movement coordination state
    int target_left_steps;
    int target_right_steps;
    int current_left_steps;
    int current_right_steps;
    bool movement_active;
    
    // Step counting for future position tracking (Phase 2)
    long left_steps_total;
    long right_steps_total;
    
    // Robot configuration
    RobotConfig robot_config;
    
public:
    // === INITIALIZATION ===
    void begin(const RobotConfig& config);
    
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
    bool isMovementComplete();
    void stopAllMotors();
};

#endif // TERRAPEN_ROBOT_H