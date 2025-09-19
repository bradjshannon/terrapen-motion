#include "TerraPenRobot.h"

/**
 * Initialize the robot with hardware configuration from global config
 */
void TerraPenRobot::begin() {
    // Initialize hardware drivers using global configuration
    left_motor.begin(g_config.hardware.motor_l_pins[0], g_config.hardware.motor_l_pins[1], 
                     g_config.hardware.motor_l_pins[2], g_config.hardware.motor_l_pins[3]);
    right_motor.begin(g_config.hardware.motor_r_pins[0], g_config.hardware.motor_r_pins[1], 
                      g_config.hardware.motor_r_pins[2], g_config.hardware.motor_r_pins[3]);
    pen_servo.begin(g_config.hardware.servo_pin);
    
    // Set motor speeds based on configuration
    float speed_sps = 1000000.0 / g_config.hardware.step_delay_us;
    left_motor.setSpeed(speed_sps);
    right_motor.setSpeed(speed_sps);
    
    // Initialize state
    state = IDLE;
    pen_is_down = false;
    movement_active = false;
    
    // Initialize movement tracking
    target_left_steps = 0;
    target_right_steps = 0;
    current_left_steps = 0;
    current_right_steps = 0;
    
    // Initialize step counters
    left_steps_total = 0;
    right_steps_total = 0;
    
    // Set pen to up position initially
    pen_servo.setAngle(g_config.hardware.servo_pen_up_angle);
}

/**
 * Move both motors forward for specified number of steps
 */
bool TerraPenRobot::moveForward(int steps) {
    if (isBusy() || steps <= 0) {
        return false;
    }
    
    // Set movement targets
    target_left_steps = steps;
    target_right_steps = steps;
    current_left_steps = 0;
    current_right_steps = 0;
    movement_active = true;
    
    setState(MOVING);
    return true;
}

/**
 * Move both motors backward for specified number of steps
 */
bool TerraPenRobot::moveBackward(int steps) {
    if (isBusy() || steps <= 0) {
        return false;
    }
    
    // Set movement targets (negative for backward)
    target_left_steps = -steps;
    target_right_steps = -steps;
    current_left_steps = 0;
    current_right_steps = 0;
    movement_active = true;
    
    setState(MOVING);
    return true;
}

/**
 * Turn left by moving right motor forward and left motor backward
 */
bool TerraPenRobot::turnLeft(int steps) {
    if (isBusy() || steps <= 0) {
        return false;
    }
    
    // For differential drive: left turn = right motor forward, left motor backward
    target_left_steps = -steps;
    target_right_steps = steps;
    current_left_steps = 0;
    current_right_steps = 0;
    movement_active = true;
    
    setState(MOVING);
    return true;
}

/**
 * Turn right by moving left motor forward and right motor backward
 */
bool TerraPenRobot::turnRight(int steps) {
    if (isBusy() || steps <= 0) {
        return false;
    }
    
    // For differential drive: right turn = left motor forward, right motor backward
    target_left_steps = steps;
    target_right_steps = -steps;
    current_left_steps = 0;
    current_right_steps = 0;
    movement_active = true;
    
    setState(MOVING);
    return true;
}

/**
 * Raise the pen
 */
void TerraPenRobot::penUp() {
    pen_servo.setAngle(g_config.hardware.servo_pen_up_angle);
    pen_is_down = false;
}

/**
 * Lower the pen
 */
void TerraPenRobot::penDown() {
    pen_servo.setAngle(g_config.hardware.servo_pen_down_angle);
    pen_is_down = true;
}

/**
 * Check if pen is currently down
 */
bool TerraPenRobot::isPenDown() const {
    return pen_is_down;
}

/**
 * Get current robot state
 */
RobotState TerraPenRobot::getState() const {
    return state;
}

/**
 * Check if robot is currently busy (moving or error state)
 */
bool TerraPenRobot::isBusy() const {
    return (state == MOVING) || (state == ERROR) || (state == EMERGENCY_STOP);
}

/**
 * Emergency stop - immediately halt all movement
 */
void TerraPenRobot::emergencyStop() {
    stopAllMotors();
    movement_active = false;
    setState(EMERGENCY_STOP);
}

/**
 * Clear error state and return to idle
 */
void TerraPenRobot::clearError() {
    if (state == ERROR || state == EMERGENCY_STOP) {
        stopAllMotors();
        movement_active = false;
        setState(IDLE);
    }
}

/**
 * Get total left motor steps (for position tracking in Phase 2)
 */
long TerraPenRobot::getLeftStepsTotal() const {
    return left_steps_total;
}

/**
 * Get total right motor steps (for position tracking in Phase 2)
 */
long TerraPenRobot::getRightStepsTotal() const {
    return right_steps_total;
}

/**
 * Reset step counters (for calibration)
 */
void TerraPenRobot::resetStepCounts() {
    left_steps_total = 0;
    right_steps_total = 0;
}

/**
 * Main update function - call every loop iteration
 * Coordinates the non-blocking hardware drivers
 */
void TerraPenRobot::update() {
    // Update servo driver for smooth movements
    pen_servo.update();
    
    // Execute movement if active
    if (movement_active && state == MOVING) {
        executeMovement();
        
        // Check if movement is complete
        if (isMovementComplete()) {
            movement_active = false;
            setState(IDLE);
        }
    }
}

/**
 * Execute coordinated movement using non-blocking drivers
 */
void TerraPenRobot::executeMovement() {
    bool left_step_taken = false;
    bool right_step_taken = false;
    
    // Handle left motor
    if (current_left_steps != target_left_steps && left_motor.isReady()) {
        if (target_left_steps > 0) {
            // Forward
            if (current_left_steps < target_left_steps) {
                if (left_motor.stepForward()) {
                    current_left_steps++;
                    left_steps_total++;
                    left_step_taken = true;
                }
            }
        } else {
            // Backward
            if (current_left_steps > target_left_steps) {
                if (left_motor.stepBackward()) {
                    current_left_steps--;
                    left_steps_total--;
                    left_step_taken = true;
                }
            }
        }
    }
    
    // Handle right motor
    if (current_right_steps != target_right_steps && right_motor.isReady()) {
        if (target_right_steps > 0) {
            // Forward
            if (current_right_steps < target_right_steps) {
                if (right_motor.stepForward()) {
                    current_right_steps++;
                    right_steps_total++;
                    right_step_taken = true;
                }
            }
        } else {
            // Backward
            if (current_right_steps > target_right_steps) {
                if (right_motor.stepBackward()) {
                    current_right_steps--;
                    right_steps_total--;
                    right_step_taken = true;
                }
            }
        }
    }
}

/**
 * Check if current movement is complete
 */
bool TerraPenRobot::isMovementComplete() {
    return (current_left_steps == target_left_steps) && 
           (current_right_steps == target_right_steps);
}

/**
 * Stop all motors immediately
 */
void TerraPenRobot::stopAllMotors() {
    left_motor.release();
    right_motor.release();
}

/**
 * Set robot state with validation
 */
void TerraPenRobot::setState(RobotState new_state) {
    // Add state transition validation if needed
    state = new_state;
}