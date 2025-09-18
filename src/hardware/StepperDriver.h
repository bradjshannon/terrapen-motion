#ifndef STEPPER_DRIVER_H
#define STEPPER_DRIVER_H

#include <Arduino.h>

/**
 * StepperDriver - Controls 28BYJ-48 stepper motors via ULN2803A driver
 * 
 * Features:
 * - Half-step sequence for smooth operation and higher resolution
 * - Non-blocking stepping with precise timing control
 * - Configurable speed control
 * - Motor hold/release for power management
 * 
 * Usage:
 *   StepperDriver motor;
 *   motor.begin(2, 3, 4, 5);  // IN1, IN2, IN3, IN4 pins
 *   motor.setSpeed(100);      // 100 steps per second
 *   
 *   // In main loop:
 *   if (motor.stepForward()) {
 *     // Step was taken
 *   }
 */
class StepperDriver {
private:
    // Pin assignments
    int pins[4];                    // IN1, IN2, IN3, IN4
    
    // Stepping state
    int current_phase;              // 0-7 for half-step sequence
    unsigned long last_step_us;     // Timestamp of last step (microseconds)
    unsigned long step_interval_us; // Microseconds between steps
    
    // 28BYJ-48 half-step sequence (8 steps per full step)
    // Each row represents [IN1, IN2, IN3, IN4] states
    static const int PHASE_SEQUENCE[8][4];
    
    // Motor state
    bool initialized;
    bool motor_enabled;
    
public:
    // === CONSTRUCTOR ===
    
    /**
     * Default constructor
     */
    StepperDriver();
    
    // === INITIALIZATION ===
    
    /**
     * Initialize stepper driver with pin assignments
     * @param in1 Arduino pin connected to IN1 of ULN2803A
     * @param in2 Arduino pin connected to IN2 of ULN2803A  
     * @param in3 Arduino pin connected to IN3 of ULN2803A
     * @param in4 Arduino pin connected to IN4 of ULN2803A
     */
    void begin(int in1, int in2, int in3, int in4);
    
    // === SPEED CONTROL ===
    
    /**
     * Set stepping speed
     * @param steps_per_sec Desired steps per second (typical range: 1-500)
     */
    void setSpeed(float steps_per_sec);
    
    /**
     * Get current speed setting
     * @return Current steps per second
     */
    float getSpeed() const;
    
    // === STEPPING CONTROL ===
    
    /**
     * Attempt to step forward (non-blocking)
     * @return true if a step was actually taken, false if not ready yet
     */
    bool stepForward();
    
    /**
     * Attempt to step backward (non-blocking)
     * @return true if a step was actually taken, false if not ready yet
     */
    bool stepBackward();
    
    /**
     * Take immediate step (blocking)
     * @param direction 1 for forward, -1 for backward
     */
    void stepNow(int direction);
    
    /**
     * Check if ready for next step
     * @return true if enough time has passed for next step
     */
    bool isReady() const;
    
    // === MOTOR CONTROL ===
    
    /**
     * Energize coils to hold current position
     * Prevents shaft rotation but consumes power
     */
    void hold();
    
    /**
     * Turn off all coils to save power
     * Motor can be rotated freely by hand
     */
    void release();
    
    /**
     * Check if motor is currently energized
     * @return true if motor is holding position
     */
    bool isHolding() const;
    
    // === STATE QUERIES ===
    
    /**
     * Get current phase in stepping sequence
     * @return Phase number (0-7)
     */
    int getCurrentPhase() const;
    
    /**
     * Check if driver is properly initialized
     * @return true if begin() has been called successfully
     */
    bool isInitialized() const;
    
private:
    // === INTERNAL HELPERS ===
    
    /**
     * Apply current phase to motor pins
     */
    void applyPhase();
    
    /**
     * Update current phase for given direction
     * @param direction 1 for forward, -1 for backward
     */
    void updatePhase(int direction);
    
    /**
     * Set all motor pins to OFF state
     */
    void clearPins();
};

#endif // STEPPER_DRIVER_H