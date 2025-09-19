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
    
    // Initialize position tracking (Phase 2)
    current_x = 0.0;
    current_y = 0.0;
    current_angle = 0.0;
    coordinate_movement = false;
    movement_speed_mms = 15.0;
    target_x = 0.0;
    target_y = 0.0;
    
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
    coordinate_movement = false;  // Step-based movement
    
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
    coordinate_movement = false;  // Step-based movement
    
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
    coordinate_movement = false;  // Step-based movement
    
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
    coordinate_movement = false;  // Step-based movement
    
    setState(MOVING);
    return true;
}

/**
 * Move to specific coordinates with pen up
 */
bool TerraPenRobot::moveTo(float x, float y, float speed_mms) {
    if (isBusy() || !isValidPosition(x, y) || speed_mms <= 0) {
        return false;
    }
    
    // Ensure pen is up for movement
    penUp();
    
    // Set coordinate movement target
    target_x = x;
    target_y = y;
    movement_speed_mms = speed_mms;
    coordinate_movement = true;
    movement_active = true;
    
    setState(MOVING);
    return true;
}

/**
 * Draw line to specific coordinates with pen down
 */
bool TerraPenRobot::drawTo(float x, float y, float speed_mms) {
    if (isBusy() || !isValidPosition(x, y) || speed_mms <= 0) {
        return false;
    }
    
    // Ensure pen is down for drawing
    penDown();
    
    // Set coordinate movement target
    target_x = x;
    target_y = y;
    movement_speed_mms = speed_mms;
    coordinate_movement = true;
    movement_active = true;
    
    setState(MOVING);
    return true;
}

/**
 * Move by relative offset with pen up
 */
bool TerraPenRobot::moveBy(float dx, float dy, float speed_mms) {
    return moveTo(current_x + dx, current_y + dy, speed_mms);
}

/**
 * Draw by relative offset with pen down
 */
bool TerraPenRobot::drawBy(float dx, float dy, float speed_mms) {
    return drawTo(current_x + dx, current_y + dy, speed_mms);
}

/**
 * Turn to absolute angle
 */
bool TerraPenRobot::turnTo(float angle_radians, float speed_rad_s) {
    if (isBusy() || speed_rad_s <= 0) {
        return false;
    }
    
    // Calculate required turn angle
    float delta_angle = angle_radians - current_angle;
    
    // Normalize to [-PI, PI]
    while (delta_angle > PI) delta_angle -= 2 * PI;
    while (delta_angle < -PI) delta_angle += 2 * PI;
    
    return turnBy(delta_angle, speed_rad_s);
}

/**
 * Turn by relative angle
 */
bool TerraPenRobot::turnBy(float delta_angle, float speed_rad_s) {
    if (isBusy() || speed_rad_s <= 0) {
        return false;
    }
    
    // Calculate steps needed for rotation
    int left_steps, right_steps;
    calculateSteps(0.0, delta_angle, left_steps, right_steps);
    
    // Set movement targets
    target_left_steps = left_steps;
    target_right_steps = right_steps;
    current_left_steps = 0;
    current_right_steps = 0;
    movement_active = true;
    coordinate_movement = false;  // Step-based movement
    
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
 * Get current position and orientation
 */
Position TerraPenRobot::getCurrentPosition() const {
    return Position(current_x, current_y, current_angle);
}

/**
 * Reset position tracking (for calibration)
 */
void TerraPenRobot::resetPosition(float x, float y, float angle) {
    current_x = x;
    current_y = y;
    current_angle = angle;
    
    // Normalize angle to [-PI, PI]
    while (current_angle > PI) current_angle -= 2 * PI;
    while (current_angle < -PI) current_angle += 2 * PI;
    
    // Reset step counters to maintain consistency
    resetStepCounts();
}

/**
 * Check if robot is at target position
 */
bool TerraPenRobot::isAtTarget() const {
    if (!coordinate_movement) {
        return isMovementComplete();
    }
    return isAtTargetPosition();
}

/**
 * Check if position is within workspace boundaries
 */
bool TerraPenRobot::isValidPosition(float x, float y) const {
    return (x >= g_config.hardware.workspace_min_x && x <= g_config.hardware.workspace_max_x &&
            y >= g_config.hardware.workspace_min_y && y <= g_config.hardware.workspace_max_y);
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
        if (coordinate_movement) {
            executeCoordinateMovement();
        } else {
            executeMovement();
        }
        
        // Check if movement is complete
        if ((coordinate_movement && isAtTargetPosition()) || 
            (!coordinate_movement && isMovementComplete())) {
            movement_active = false;
            coordinate_movement = false;
            setState(IDLE);
        }
    }
    
    // Update position estimate based on step changes
    updatePositionEstimate();
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
bool TerraPenRobot::isMovementComplete() const {
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

/**
 * Calculate motor steps needed for given distance and angle change
 * Using differential drive kinematics
 */
void TerraPenRobot::calculateSteps(float distance_mm, float angle_diff, int& left_steps, int& right_steps) {
    // Get hardware parameters
    float wheel_diameter = g_config.hardware.wheel_diameter_mm;
    float wheelbase = g_config.hardware.wheelbase_mm;
    int steps_per_rev = g_config.hardware.steps_per_revolution;
    
    // Calculate wheel circumference
    float wheel_circumference = PI * wheel_diameter;
    
    // For pure rotation (distance = 0):
    // Arc length each wheel travels = angle_diff * wheelbase / 2
    // For pure translation (angle_diff = 0):
    // Both wheels travel the same distance
    
    // Calculate distance each wheel needs to travel
    float arc_length = angle_diff * wheelbase / 2.0;
    float left_distance = distance_mm - arc_length;   // Left wheel travels less for right turn
    float right_distance = distance_mm + arc_length;  // Right wheel travels more for right turn
    
    // Convert distances to steps
    left_steps = (int)round((left_distance / wheel_circumference) * steps_per_rev);
    right_steps = (int)round((right_distance / wheel_circumference) * steps_per_rev);
}

/**
 * Convert motor steps back to distance and angle change
 * Inverse kinematics for position estimation
 */
void TerraPenRobot::stepsToMovement(int left_steps, int right_steps, float& distance, float& angle_change) {
    // Get hardware parameters
    float wheel_diameter = g_config.hardware.wheel_diameter_mm;
    float wheelbase = g_config.hardware.wheelbase_mm;
    int steps_per_rev = g_config.hardware.steps_per_revolution;
    
    // Calculate wheel circumference
    float wheel_circumference = PI * wheel_diameter;
    
    // Convert steps to distances
    float left_distance = (float(left_steps) / steps_per_rev) * wheel_circumference;
    float right_distance = (float(right_steps) / steps_per_rev) * wheel_circumference;
    
    // Calculate robot movement
    distance = (left_distance + right_distance) / 2.0;  // Average distance
    angle_change = (right_distance - left_distance) / wheelbase;  // Differential creates rotation
}

/**
 * Update position estimate based on step counts since last update
 */
void TerraPenRobot::updatePositionEstimate() {
    static long last_left_steps = 0;
    static long last_right_steps = 0;
    
    // Calculate step changes since last update
    int delta_left = left_steps_total - last_left_steps;
    int delta_right = right_steps_total - last_right_steps;
    
    // Only update if there were step changes
    if (delta_left != 0 || delta_right != 0) {
        float distance, angle_change;
        stepsToMovement(delta_left, delta_right, distance, angle_change);
        
        // Update position based on movement in current direction
        current_x += distance * sin(current_angle);
        current_y += distance * cos(current_angle);
        current_angle += angle_change;
        
        // Normalize angle to [-PI, PI]
        while (current_angle > PI) current_angle -= 2 * PI;
        while (current_angle < -PI) current_angle += 2 * PI;
        
        // Update step tracking
        last_left_steps = left_steps_total;
        last_right_steps = right_steps_total;
    }
}

/**
 * Execute coordinate-based movement
 */
void TerraPenRobot::executeCoordinateMovement() {
    // Calculate distance and angle to target
    float dx = target_x - current_x;
    float dy = target_y - current_y;
    float distance_to_target = sqrt(dx * dx + dy * dy);
    
    // Check if we're close enough to target (within 0.5mm)
    if (distance_to_target < 0.5) {
        return;  // Close enough, movement will complete
    }
    
    // Calculate required angle to target
    float required_angle = atan2(dx, dy);
    float angle_diff = required_angle - current_angle;
    
    // Normalize angle difference to [-PI, PI]
    while (angle_diff > PI) angle_diff -= 2 * PI;
    while (angle_diff < -PI) angle_diff += 2 * PI;
    
    // If we need to turn significantly (> 5 degrees), turn first
    if (abs(angle_diff) > 0.087) {  // 5 degrees in radians
        // Calculate steps for pure rotation
        int left_steps, right_steps;
        calculateSteps(0.0, angle_diff, left_steps, right_steps);
        
        // Set rotation targets
        target_left_steps = left_steps;
        target_right_steps = right_steps;
        current_left_steps = 0;
        current_right_steps = 0;
        
        // Execute step-based movement
        executeMovement();
    } else {
        // Move forward toward target
        float step_distance = min(distance_to_target, 1.0);  // Max 1mm per step
        int left_steps, right_steps;
        calculateSteps(step_distance, 0.0, left_steps, right_steps);
        
        // Set movement targets
        target_left_steps = left_steps;
        target_right_steps = right_steps;
        current_left_steps = 0;
        current_right_steps = 0;
        
        // Execute step-based movement
        executeMovement();
    }
}

/**
 * Check if robot is at target coordinates
 */
bool TerraPenRobot::isAtTargetPosition() const {
    float dx = target_x - current_x;
    float dy = target_y - current_y;
    float distance = sqrt(dx * dx + dy * dy);
    return distance < 0.5;  // Within 0.5mm tolerance
}