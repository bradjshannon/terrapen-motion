#include "StepperDriver.h"

// 28BYJ-48 half-step sequence for smooth operation
// Phase sequence: [IN1, IN2, IN3, IN4]
const int StepperDriver::PHASE_SEQUENCE[8][4] = {
    {1, 0, 0, 0},  // Phase 0
    {1, 1, 0, 0},  // Phase 1
    {0, 1, 0, 0},  // Phase 2
    {0, 1, 1, 0},  // Phase 3
    {0, 0, 1, 0},  // Phase 4
    {0, 0, 1, 1},  // Phase 5
    {0, 0, 0, 1},  // Phase 6
    {1, 0, 0, 1}   // Phase 7
};

StepperDriver::StepperDriver() : 
    current_phase(0),
    last_step_us(0),
    step_interval_us(10000),  // Default: 100 steps/sec
    initialized(false),
    motor_enabled(false)
{
    // Initialize pin array
    for (int i = 0; i < 4; i++) {
        pins[i] = -1;
    }
}

void StepperDriver::begin(int in1, int in2, int in3, int in4) {
    // Store pin assignments
    pins[0] = in1;
    pins[1] = in2;
    pins[2] = in3;
    pins[3] = in4;
    
    // Configure pins as outputs
    for (int i = 0; i < 4; i++) {
        pinMode(pins[i], OUTPUT);
        digitalWrite(pins[i], LOW);
    }
    
    // Initialize state
    current_phase = 0;
    last_step_us = micros();
    motor_enabled = false;
    initialized = true;
    
    // Apply initial phase (but motor is released)
    clearPins();
}

void StepperDriver::setSpeed(float steps_per_sec) {
    if (steps_per_sec <= 0) {
        step_interval_us = 1000000;  // Very slow (1 step/sec)
        return;
    }
    
    // Calculate microseconds between steps
    step_interval_us = (unsigned long)(1000000.0 / steps_per_sec);
    
    // Clamp to reasonable limits
    if (step_interval_us < 1000) {      // Max ~1000 steps/sec
        step_interval_us = 1000;
    }
    if (step_interval_us > 1000000) {   // Min 1 step/sec
        step_interval_us = 1000000;
    }
}

float StepperDriver::getSpeed() const {
    if (step_interval_us == 0) return 0;
    return 1000000.0 / step_interval_us;
}

bool StepperDriver::stepForward() {
    if (!initialized) return false;
    
    // Check if enough time has passed
    if (!isReady()) return false;
    
    // Take the step
    updatePhase(1);  // Forward direction
    applyPhase();
    motor_enabled = true;
    last_step_us = micros();
    
    return true;
}

bool StepperDriver::stepBackward() {
    if (!initialized) return false;
    
    // Check if enough time has passed
    if (!isReady()) return false;
    
    // Take the step
    updatePhase(-1);  // Backward direction
    applyPhase();
    motor_enabled = true;
    last_step_us = micros();
    
    return true;
}

void StepperDriver::stepNow(int direction) {
    if (!initialized) return;
    
    // Take immediate step regardless of timing
    updatePhase(direction);
    applyPhase();
    motor_enabled = true;
    last_step_us = micros();
}

bool StepperDriver::isReady() const {
    if (!initialized) return false;
    
    unsigned long current_us = micros();
    
    // Handle micros() overflow (occurs every ~70 minutes)
    if (current_us < last_step_us) {
        // Overflow occurred, assume we're ready
        return true;
    }
    
    return (current_us - last_step_us) >= step_interval_us;
}

void StepperDriver::hold() {
    if (!initialized) return;
    
    motor_enabled = true;
    applyPhase();
}

void StepperDriver::release() {
    if (!initialized) return;
    
    motor_enabled = false;
    clearPins();
}

bool StepperDriver::isHolding() const {
    return motor_enabled;
}

int StepperDriver::getCurrentPhase() const {
    return current_phase;
}

bool StepperDriver::isInitialized() const {
    return initialized;
}

// === PRIVATE METHODS ===

void StepperDriver::applyPhase() {
    if (!initialized || !motor_enabled) {
        clearPins();
        return;
    }
    
    // Apply current phase to pins
    for (int i = 0; i < 4; i++) {
        digitalWrite(pins[i], PHASE_SEQUENCE[current_phase][i]);
    }
}

void StepperDriver::updatePhase(int direction) {
    if (direction > 0) {
        // Forward: increment phase
        current_phase = (current_phase + 1) % 8;
    } else if (direction < 0) {
        // Backward: decrement phase
        current_phase = (current_phase - 1 + 8) % 8;
    }
    // direction == 0: no change
}

void StepperDriver::clearPins() {
    if (!initialized) return;
    
    for (int i = 0; i < 4; i++) {
        digitalWrite(pins[i], LOW);
    }
}