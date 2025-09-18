#include "ServoDriver.h"

ServoDriver::ServoDriver() :
    pin(-1),
    current_angle(DEFAULT_ANGLE),
    target_angle(DEFAULT_ANGLE),
    start_angle(DEFAULT_ANGLE),
    move_start_time(0),
    move_duration(0),
    is_moving(false),
    initialized(false)
{
}

void ServoDriver::begin(int servo_pin, int initial_angle) {
    pin = servo_pin;
    
    // Validate and set initial angle
    initial_angle = constrainAngle(initial_angle);
    current_angle = initial_angle;
    target_angle = initial_angle;
    start_angle = initial_angle;
    
    // Attach servo and set initial position
    servo.attach(pin);
    delay(10);  // Small delay for servo library initialization
    writeAngle(current_angle);
    
    // Initialize state
    is_moving = false;
    initialized = true;
    
    // Give servo time to reach initial position
    delay(100);
}

void ServoDriver::setAngle(int degrees) {
    if (!initialized) return;
    
    degrees = constrainAngle(degrees);
    
    // Stop any ongoing movement
    is_moving = false;
    
    // Set position immediately
    current_angle = degrees;
    target_angle = degrees;
    writeAngle(degrees);
}

void ServoDriver::sweepTo(int degrees, unsigned long duration_ms) {
    if (!initialized) return;
    
    degrees = constrainAngle(degrees);
    
    // If already at target, no movement needed
    if (degrees == current_angle) {
        is_moving = false;
        target_angle = degrees;
        return;
    }
    
    // Set up smooth movement
    start_angle = current_angle;
    target_angle = degrees;
    move_start_time = millis();
    move_duration = duration_ms;
    is_moving = true;
    
    // Ensure minimum duration to prevent jitter
    if (move_duration < 10) {
        move_duration = 10;
    }
}

int ServoDriver::getCurrentAngle() const {
    return current_angle;
}

int ServoDriver::getTargetAngle() const {
    return target_angle;
}

bool ServoDriver::isMoving() const {
    return is_moving;
}

float ServoDriver::getProgress() const {
    if (!is_moving || move_duration == 0) {
        return 1.0;
    }
    
    unsigned long elapsed = millis() - move_start_time;
    float progress = (float)elapsed / (float)move_duration;
    
    // Clamp to valid range
    if (progress > 1.0) progress = 1.0;
    if (progress < 0.0) progress = 0.0;
    
    return progress;
}

void ServoDriver::stop() {
    if (!initialized) return;
    
    // Stop movement at current position
    is_moving = false;
    target_angle = current_angle;
}

void ServoDriver::update() {
    if (!initialized || !is_moving) return;
    
    float progress = getProgress();
    
    if (progress >= 1.0) {
        // Movement complete
        current_angle = target_angle;
        is_moving = false;
        writeAngle(current_angle);
    } else {
        // Calculate intermediate position
        int new_angle = interpolateAngle(progress);
        if (new_angle != current_angle) {
            current_angle = new_angle;
            writeAngle(current_angle);
        }
    }
}

bool ServoDriver::isInitialized() const {
    return initialized;
}

void ServoDriver::detach() {
    if (!initialized) return;
    
    servo.detach();
    initialized = false;
    is_moving = false;
}

bool ServoDriver::isAttached() const {
    return initialized && servo.attached();
}

// === PRIVATE METHODS ===

int ServoDriver::constrainAngle(int angle) const {
    if (angle < MIN_ANGLE) return MIN_ANGLE;
    if (angle > MAX_ANGLE) return MAX_ANGLE;
    return angle;
}

void ServoDriver::writeAngle(int angle) {
    if (!initialized) return;
    
    angle = constrainAngle(angle);
    servo.write(angle);
}

int ServoDriver::interpolateAngle(float progress) const {
    // Linear interpolation between start and target angles
    float diff = target_angle - start_angle;
    float interpolated = start_angle + (diff * progress);
    
    return (int)(interpolated + 0.5);  // Round to nearest integer
}