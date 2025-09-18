#ifndef POSITION_H
#define POSITION_H

#include <Arduino.h>
#include <math.h>

/**
 * Position - Robot position and orientation in 2D space
 * 
 * Represents the robot's location (x, y) in millimeters and 
 * orientation (angle) in radians within the coordinate system.
 * 
 * Coordinate System:
 * - Origin (0,0) is the robot's starting position
 * - X-axis: positive right, negative left
 * - Y-axis: positive forward, negative backward  
 * - Angle: 0 = facing positive Y, positive rotation = counterclockwise
 * 
 * Usage:
 *   Position pos(10.0, 20.0, PI/4);  // x=10mm, y=20mm, facing northeast
 *   float dist = pos.distanceTo(other_pos);
 *   Position new_pos = pos.offsetBy(5.0, 0.0);
 */
struct Position {
    // === POSITION DATA ===
    
    /** X coordinate in millimeters */
    float x;
    
    /** Y coordinate in millimeters */
    float y;
    
    /** Orientation angle in radians (0 = facing +Y, + = counterclockwise) */
    float angle;
    
    // === CONSTRUCTORS ===
    
    /**
     * Default constructor (origin position)
     */
    Position() : x(0), y(0), angle(0) {}
    
    /**
     * Constructor with position and orientation
     * @param x_mm X coordinate in millimeters
     * @param y_mm Y coordinate in millimeters
     * @param angle_rad Orientation in radians
     */
    Position(float x_mm, float y_mm, float angle_rad = 0) : 
        x(x_mm), y(y_mm), angle(angle_rad) {}
    
    // === DISTANCE AND DIRECTION ===
    
    /**
     * Calculate straight-line distance to another position
     * @param other Target position
     * @return Distance in millimeters
     */
    float distanceTo(const Position& other) const {
        float dx = other.x - x;
        float dy = other.y - y;
        return sqrt(dx * dx + dy * dy);
    }
    
    /**
     * Calculate angle to another position (global coordinates)
     * @param other Target position
     * @return Angle in radians from current position to target
     */
    float angleTo(const Position& other) const {
        float dx = other.x - x;
        float dy = other.y - y;
        return atan2(dx, dy);  // Note: atan2(x, y) for our coordinate system
    }
    
    /**
     * Calculate relative angle to turn to face another position
     * @param other Target position
     * @return Angle difference in radians (-PI to PI)
     */
    float turnAngleTo(const Position& other) const {
        float target_angle = angleTo(other);
        float turn_angle = target_angle - angle;
        
        // Normalize to [-PI, PI]
        while (turn_angle > PI) turn_angle -= 2 * PI;
        while (turn_angle < -PI) turn_angle += 2 * PI;
        
        return turn_angle;
    }
    
    // === POSITION MANIPULATION ===
    
    /**
     * Create new position offset by given amounts
     * @param dx Change in X (millimeters)
     * @param dy Change in Y (millimeters)
     * @return New position
     */
    Position offsetBy(float dx, float dy) const {
        return Position(x + dx, y + dy, angle);
    }
    
    /**
     * Create new position with different orientation
     * @param new_angle New orientation in radians
     * @return New position with updated angle
     */
    Position withAngle(float new_angle) const {
        return Position(x, y, new_angle);
    }
    
    /**
     * Move forward by specified distance in current direction
     * @param distance Distance to move in millimeters
     * @return New position after forward movement
     */
    Position moveForward(float distance) const {
        float new_x = x + distance * sin(angle);
        float new_y = y + distance * cos(angle);
        return Position(new_x, new_y, angle);
    }
    
    /**
     * Rotate around current position
     * @param delta_angle Angle change in radians
     * @return New position with updated orientation
     */
    Position rotate(float delta_angle) const {
        float new_angle = angle + delta_angle;
        
        // Normalize angle to [-PI, PI]
        while (new_angle > PI) new_angle -= 2 * PI;
        while (new_angle < -PI) new_angle += 2 * PI;
        
        return Position(x, y, new_angle);
    }
    
    // === COMPARISON OPERATORS ===
    
    /**
     * Equality comparison with tolerance
     * @param other Position to compare
     * @param tolerance Maximum difference considered equal (default: 0.1mm)
     * @return true if positions are within tolerance
     */
    bool equals(const Position& other, float tolerance = 0.1) const {
        return (abs(x - other.x) <= tolerance) &&
               (abs(y - other.y) <= tolerance) &&
               (abs(angle - other.angle) <= tolerance);
    }
    
    /**
     * Check if at approximately the same location (ignoring angle)
     * @param other Position to compare
     * @param tolerance Maximum distance considered equal (default: 0.1mm)
     * @return true if within tolerance distance
     */
    bool sameLocation(const Position& other, float tolerance = 0.1) const {
        return distanceTo(other) <= tolerance;
    }
    
    // === UTILITY FUNCTIONS ===
    
    /**
     * Convert position to string for debugging
     * @return Human-readable position string
     */
    String toString() const {
        String result = "Position(";
        result += String(x, 2);
        result += ", ";
        result += String(y, 2);
        result += ", ";
        result += String(angle * 180.0 / PI, 1);  // Convert to degrees for readability
        result += "°)";
        return result;
    }
    
    /**
     * Print position to Serial
     */
    void print() const {
        Serial.print("Position: (");
        Serial.print(x, 2);
        Serial.print(", ");
        Serial.print(y, 2);
        Serial.print(") @ ");
        Serial.print(angle * 180.0 / PI, 1);
        Serial.println("°");
    }
    
    /**
     * Get angle in degrees (for display/debugging)
     * @return Angle in degrees
     */
    float getAngleDegrees() const {
        return angle * 180.0 / PI;
    }
    
    /**
     * Set angle from degrees
     * @param degrees Angle in degrees
     */
    void setAngleDegrees(float degrees) {
        angle = degrees * PI / 180.0;
    }
    
    /**
     * Check if position is valid (no NaN or infinite values)
     * @return true if all values are finite numbers
     */
    bool isValid() const {
        return isfinite(x) && isfinite(y) && isfinite(angle);
    }
    
    /**
     * Normalize angle to [-PI, PI] range
     */
    void normalizeAngle() {
        while (angle > PI) angle -= 2 * PI;
        while (angle < -PI) angle += 2 * PI;
    }
    
    // === STATIC UTILITY FUNCTIONS ===
    
    /**
     * Create position from polar coordinates (distance, angle from origin)
     * @param distance Distance from origin in millimeters
     * @param angle_rad Angle from origin in radians
     * @param orientation Robot orientation at that position (default: face origin)
     * @return Position at specified polar coordinates
     */
    static Position fromPolar(float distance, float angle_rad, float orientation = 0) {
        float x_pos = distance * sin(angle_rad);
        float y_pos = distance * cos(angle_rad);
        return Position(x_pos, y_pos, orientation);
    }
    
    /**
     * Interpolate between two positions
     * @param start Starting position
     * @param end Ending position
     * @param t Interpolation factor (0.0 = start, 1.0 = end)
     * @return Interpolated position
     */
    static Position interpolate(const Position& start, const Position& end, float t) {
        if (t <= 0) return start;
        if (t >= 1) return end;
        
        float x_interp = start.x + t * (end.x - start.x);
        float y_interp = start.y + t * (end.y - start.y);
        
        // Interpolate angle (handle wraparound)
        float angle_diff = end.angle - start.angle;
        while (angle_diff > PI) angle_diff -= 2 * PI;
        while (angle_diff < -PI) angle_diff += 2 * PI;
        float angle_interp = start.angle + t * angle_diff;
        
        return Position(x_interp, y_interp, angle_interp);
    }
};

#endif // POSITION_H