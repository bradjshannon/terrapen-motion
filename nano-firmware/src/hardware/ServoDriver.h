#ifndef SERVO_DRIVER_H
#define SERVO_DRIVER_H

#include <Arduino.h>
#include <Servo.h>

/**
 * ServoDriver - Controls servo motor with smooth movement and state tracking
 * 
 * Features:
 * - Smooth servo movement with configurable duration
 * - Non-blocking operation with state tracking
 * - Position feedback and movement status
 * - Automatic PWM signal generation
 * 
 * Usage:
 *   ServoDriver penServo;
 *   penServo.begin(9);          // Servo on pin 9
 *   penServo.sweepTo(90, 500);  // Move to 90° over 500ms
 *   
 *   // In main loop:
 *   penServo.update();          // Process smooth movement
 *   if (!penServo.isMoving()) {
 *     // Movement complete
 *   }
 */
class ServoDriver {
private:
    // Hardware
    Servo servo;
    int pin;
    
    // Position state
    int current_angle;      // Current servo angle (0-180)
    int target_angle;       // Target angle for movement
    int start_angle;        // Starting angle for smooth movement
    
    // Movement timing
    unsigned long move_start_time;  // When current movement started
    unsigned long move_duration;    // Total movement duration (ms)
    bool is_moving;                 // True if currently executing smooth movement
    
    // Initialization state
    bool initialized;
    
    // Limits and validation
    static const int MIN_ANGLE = 0;
    static const int MAX_ANGLE = 180;
    static const int DEFAULT_ANGLE = 90;
    
public:
    // === CONSTRUCTOR ===
    
    /**
     * Default constructor
     */
    ServoDriver();
    
    // === INITIALIZATION ===
    
    /**
     * Initialize servo driver
     * @param servo_pin Arduino pin connected to servo signal wire
     * @param initial_angle Starting position (default: 90°)
     */
    void begin(int servo_pin, int initial_angle = DEFAULT_ANGLE);
    
    // === POSITION CONTROL ===
    
    /**
     * Set servo angle immediately
     * @param degrees Target angle (0-180°)
     */
    void setAngle(int degrees);
    
    /**
     * Smooth movement to target angle
     * @param degrees Target angle (0-180°)
     * @param duration_ms Time for movement in milliseconds
     */
    void sweepTo(int degrees, unsigned long duration_ms);
    
    /**
     * Get current servo angle
     * @return Current angle in degrees
     */
    int getCurrentAngle() const;
    
    /**
     * Get target angle (for ongoing movements)
     * @return Target angle in degrees
     */
    int getTargetAngle() const;
    
    // === MOVEMENT STATE ===
    
    /**
     * Check if servo is currently moving
     * @return true if smooth movement is in progress
     */
    bool isMoving() const;
    
    /**
     * Get movement progress
     * @return Progress from 0.0 (start) to 1.0 (complete)
     */
    float getProgress() const;
    
    /**
     * Stop current movement immediately
     * Servo will hold current position
     */
    void stop();
    
    // === UPDATE FUNCTION ===
    
    /**
     * Update servo position (call every loop iteration)
     * Processes smooth movements and updates servo position
     */
    void update();
    
    // === UTILITY ===
    
    /**
     * Check if driver is properly initialized
     * @return true if begin() has been called successfully
     */
    bool isInitialized() const;
    
    /**
     * Detach servo to save power
     * Call begin() again to re-enable
     */
    void detach();
    
    /**
     * Check if servo is attached and active
     * @return true if servo is attached to pin
     */
    bool isAttached() const;
    
private:
    // === INTERNAL HELPERS ===
    
    /**
     * Constrain angle to valid range
     * @param angle Input angle
     * @return Clamped angle (0-180)
     */
    int constrainAngle(int angle) const;
    
    /**
     * Write angle to servo with validation
     * @param angle Target angle
     */
    void writeAngle(int angle);
    
    /**
     * Calculate intermediate angle for smooth movement
     * @param progress Movement progress (0.0 to 1.0)
     * @return Interpolated angle
     */
    int interpolateAngle(float progress) const;
};

#endif // SERVO_DRIVER_H