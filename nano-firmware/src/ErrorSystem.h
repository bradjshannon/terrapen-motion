#ifndef ERROR_SYSTEM_H
#define ERROR_SYSTEM_H

#include <Arduino.h>

/**
 * Enhanced Error Handling System
 * 
 * Provides structured error codes, detailed error messages, and error context
 * for debugging and diagnostics. Replaces simple boolean error states with
 * actionable error information.
 */

// === ERROR CODES ===

enum ErrorCode {
    ERR_NONE = 0,                    // No error
    
    // Hardware errors (1-19)
    ERR_MOTOR_TIMEOUT = 1,           // Motor didn't respond in expected time
    ERR_MOTOR_STALLED = 2,           // Motor appears to be blocked/stalled
    ERR_SERVO_TIMEOUT = 3,           // Servo didn't reach target position
    ERR_SERVO_UNRESPONSIVE = 4,      // Servo not responding to commands
    ERR_PIN_CONFLICT = 5,            // Same pin assigned to multiple functions
    ERR_INVALID_PIN = 6,             // Pin number outside valid range
    ERR_HARDWARE_INIT_FAILED = 7,    // Hardware initialization failed
    
    // Movement errors (20-39)
    ERR_INVALID_SPEED = 20,          // Speed outside hardware limits
    ERR_INVALID_STEPS = 21,          // Invalid step count (negative, zero, too large)
    ERR_MOVEMENT_BLOCKED = 22,       // Movement command while already moving
    ERR_WORKSPACE_VIOLATION = 23,    // Movement would exceed workspace limits
    ERR_INVALID_COORDINATES = 24,    // Invalid target coordinates
    ERR_MOVEMENT_TIMEOUT = 25,       // Movement took too long to complete
    
    // Configuration errors (40-59)
    ERR_INVALID_CONFIG = 40,         // Configuration validation failed
    ERR_MISSING_CONFIG = 41,         // Required configuration not provided
    ERR_CONFIG_CONFLICT = 42,        // Conflicting configuration values
    
    // System errors (60-79)
    ERR_MEMORY_ALLOCATION = 60,      // Memory allocation failed
    ERR_STACK_OVERFLOW = 61,         // Stack overflow detected
    ERR_WATCHDOG_TIMEOUT = 62,       // Watchdog timer expired
    ERR_POWER_SUPPLY = 63,           // Power supply voltage out of range
    
    // Communication errors (80-99)
    ERR_SERIAL_TIMEOUT = 80,         // Serial communication timeout
    ERR_INVALID_COMMAND = 81,        // Invalid command received
    ERR_CHECKSUM_FAILED = 82,        // Data checksum validation failed
    
    // User/logic errors (100-119)
    ERR_INVALID_STATE = 100,         // Operation not valid in current state
    ERR_NOT_INITIALIZED = 101,       // Component not properly initialized
    ERR_OPERATION_ABORTED = 102,     // Operation was aborted by user
    
    ERR_UNKNOWN = 255                // Unknown error
};

// === ERROR SEVERITY LEVELS ===

enum ErrorSeverity {
    SEVERITY_INFO = 0,     // Informational - no action required
    SEVERITY_WARNING = 1,  // Warning - operation may continue with caution
    SEVERITY_ERROR = 2,    // Error - operation failed but system can recover
    SEVERITY_CRITICAL = 3  // Critical - system should stop immediately
};

// === ERROR CONTEXT STRUCTURE ===

struct ErrorContext {
    ErrorCode code;
    ErrorSeverity severity;
    unsigned long timestamp_ms;
    String component;           // Which component reported the error
    String details;            // Human-readable error details
    String suggested_action;   // What the user should do
    
    // Technical context
    int line_number;           // Source code line (if available)
    float context_value;       // Relevant numeric value (speed, position, etc.)
    
    ErrorContext() : code(ERR_NONE), severity(SEVERITY_INFO), timestamp_ms(0), 
                    line_number(0), context_value(0.0) {}
    
    ErrorContext(ErrorCode err_code, const String& comp, const String& desc) :
        code(err_code), component(comp), details(desc), timestamp_ms(millis()),
        line_number(0), context_value(0.0) {
        severity = getDefaultSeverity(err_code);
    }
    
    // Helper to determine default severity from error code
    static ErrorSeverity getDefaultSeverity(ErrorCode code) {
        if (code == ERR_NONE) return SEVERITY_INFO;
        if (code >= ERR_MOTOR_TIMEOUT && code <= ERR_HARDWARE_INIT_FAILED) return SEVERITY_ERROR;
        if (code >= ERR_INVALID_SPEED && code <= ERR_MOVEMENT_TIMEOUT) return SEVERITY_ERROR;
        if (code >= ERR_INVALID_CONFIG && code <= ERR_CONFIG_CONFLICT) return SEVERITY_WARNING;
        if (code >= ERR_MEMORY_ALLOCATION && code <= ERR_POWER_SUPPLY) return SEVERITY_CRITICAL;
        if (code >= ERR_SERIAL_TIMEOUT && code <= ERR_CHECKSUM_FAILED) return SEVERITY_WARNING;
        if (code >= ERR_INVALID_STATE && code <= ERR_OPERATION_ABORTED) return SEVERITY_ERROR;
        return SEVERITY_ERROR;
    }
    
    // Format error for display
    String toString() const {
        String result = "[";
        result += severityToString(severity);
        result += "] ";
        result += component;
        result += " - ";
        result += getErrorName(code);
        result += ": ";
        result += details;
        
        if (context_value != 0.0) {
            result += " (value: ";
            result += String(context_value);
            result += ")";
        }
        
        if (suggested_action.length() > 0) {
            result += " | Action: ";
            result += suggested_action;
        }
        
        return result;
    }
    
    // Format as JSON for logging/debugging
    String toJson() const {
        String json = "{";
        json += "\"code\":" + String((int)code) + ",";
        json += "\"severity\":" + String((int)severity) + ",";
        json += "\"timestamp\":" + String(timestamp_ms) + ",";
        json += "\"component\":\"" + component + "\",";
        json += "\"details\":\"" + details + "\",";
        json += "\"suggested_action\":\"" + suggested_action + "\"";
        if (context_value != 0.0) {
            json += ",\"context_value\":" + String(context_value);
        }
        json += "}";
        return json;
    }
    
private:
    static String severityToString(ErrorSeverity sev) {
        switch (sev) {
            case SEVERITY_INFO: return "INFO";
            case SEVERITY_WARNING: return "WARN";
            case SEVERITY_ERROR: return "ERROR";
            case SEVERITY_CRITICAL: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }
    
    static String getErrorName(ErrorCode code) {
        switch (code) {
            case ERR_NONE: return "NO_ERROR";
            case ERR_MOTOR_TIMEOUT: return "MOTOR_TIMEOUT";
            case ERR_MOTOR_STALLED: return "MOTOR_STALLED";
            case ERR_SERVO_TIMEOUT: return "SERVO_TIMEOUT";
            case ERR_SERVO_UNRESPONSIVE: return "SERVO_UNRESPONSIVE";
            case ERR_PIN_CONFLICT: return "PIN_CONFLICT";
            case ERR_INVALID_PIN: return "INVALID_PIN";
            case ERR_HARDWARE_INIT_FAILED: return "HARDWARE_INIT_FAILED";
            case ERR_INVALID_SPEED: return "INVALID_SPEED";
            case ERR_INVALID_STEPS: return "INVALID_STEPS";
            case ERR_MOVEMENT_BLOCKED: return "MOVEMENT_BLOCKED";
            case ERR_WORKSPACE_VIOLATION: return "WORKSPACE_VIOLATION";
            case ERR_INVALID_COORDINATES: return "INVALID_COORDINATES";
            case ERR_MOVEMENT_TIMEOUT: return "MOVEMENT_TIMEOUT";
            case ERR_INVALID_CONFIG: return "INVALID_CONFIG";
            case ERR_MISSING_CONFIG: return "MISSING_CONFIG";
            case ERR_CONFIG_CONFLICT: return "CONFIG_CONFLICT";
            case ERR_MEMORY_ALLOCATION: return "MEMORY_ALLOCATION";
            case ERR_STACK_OVERFLOW: return "STACK_OVERFLOW";
            case ERR_WATCHDOG_TIMEOUT: return "WATCHDOG_TIMEOUT";
            case ERR_POWER_SUPPLY: return "POWER_SUPPLY";
            case ERR_SERIAL_TIMEOUT: return "SERIAL_TIMEOUT";
            case ERR_INVALID_COMMAND: return "INVALID_COMMAND";
            case ERR_CHECKSUM_FAILED: return "CHECKSUM_FAILED";
            case ERR_INVALID_STATE: return "INVALID_STATE";
            case ERR_NOT_INITIALIZED: return "NOT_INITIALIZED";
            case ERR_OPERATION_ABORTED: return "OPERATION_ABORTED";
            default: return "UNKNOWN_ERROR";
        }
    }
};

// === ERROR MANAGER CLASS ===

class ErrorManager {
private:
    static const int MAX_ERROR_HISTORY = 10;
    ErrorContext error_history[MAX_ERROR_HISTORY];
    int history_head;
    int history_count;
    ErrorContext current_error;
    bool error_reporting_enabled;
    
public:
    ErrorManager() : history_head(0), history_count(0), error_reporting_enabled(true) {
        current_error = ErrorContext();
    }
    
    // === ERROR REPORTING ===
    
    void reportError(ErrorCode code, const String& component, const String& details) {
        ErrorContext error(code, component, details);
        setCurrentError(error);
    }
    
    void reportError(const ErrorContext& error) {
        setCurrentError(error);
    }
    
    // Convenience methods for common error types
    void reportMotorTimeout(const String& motor_name, unsigned long timeout_ms) {
        ErrorContext error(ERR_MOTOR_TIMEOUT, motor_name, 
                          "Motor failed to respond after " + String(timeout_ms) + "ms");
        error.suggested_action = "Check motor connections and power supply";
        error.context_value = timeout_ms;
        setCurrentError(error);
    }
    
    void reportInvalidSpeed(const String& component, float requested_speed, float max_speed) {
        ErrorContext error(ERR_INVALID_SPEED, component, 
                          "Requested speed " + String(requested_speed) + " exceeds maximum " + String(max_speed));
        error.suggested_action = "Reduce speed to within hardware limits";
        error.context_value = requested_speed;
        setCurrentError(error);
    }
    
    void reportMovementBlocked(const String& component) {
        ErrorContext error(ERR_MOVEMENT_BLOCKED, component, 
                          "Cannot start movement while another movement is active");
        error.suggested_action = "Wait for current movement to complete";
        setCurrentError(error);
    }
    
    // === ERROR QUERIES ===
    
    bool hasError() const {
        return current_error.code != ERR_NONE;
    }
    
    bool hasCriticalError() const {
        return hasError() && current_error.severity == SEVERITY_CRITICAL;
    }
    
    ErrorCode getCurrentErrorCode() const {
        return current_error.code;
    }
    
    ErrorContext getCurrentError() const {
        return current_error;
    }
    
    String getCurrentErrorString() const {
        return current_error.toString();
    }
    
    String getCurrentErrorJson() const {
        return current_error.toJson();
    }
    
    // === ERROR MANAGEMENT ===
    
    void clearError() {
        current_error = ErrorContext();
    }
    
    void clearErrorIfCode(ErrorCode code) {
        if (current_error.code == code) {
            clearError();
        }
    }
    
    // === ERROR HISTORY ===
    
    int getErrorHistoryCount() const {
        return history_count;
    }
    
    ErrorContext getHistoryError(int index) const {
        if (index >= 0 && index < history_count) {
            int actual_index = (history_head - index - 1 + MAX_ERROR_HISTORY) % MAX_ERROR_HISTORY;
            return error_history[actual_index];
        }
        return ErrorContext();
    }
    
    void printErrorHistory() const {
        if (!error_reporting_enabled) return;
        
        Serial.println("=== Error History ===");
        for (int i = 0; i < history_count; i++) {
            ErrorContext error = getHistoryError(i);
            Serial.print(i);
            Serial.print(": ");
            Serial.println(error.toString());
        }
        Serial.println("====================");
    }
    
    // === CONFIGURATION ===
    
    void setErrorReporting(bool enabled) {
        error_reporting_enabled = enabled;
    }
    
    bool isErrorReportingEnabled() const {
        return error_reporting_enabled;
    }
    
private:
    void setCurrentError(const ErrorContext& error) {
        // Set current error
        current_error = error;
        
        // Add to history
        error_history[history_head] = error;
        history_head = (history_head + 1) % MAX_ERROR_HISTORY;
        if (history_count < MAX_ERROR_HISTORY) {
            history_count++;
        }
        
        // Print error if reporting enabled
        if (error_reporting_enabled && error.severity >= SEVERITY_WARNING) {
            Serial.println("ERROR: " + error.toString());
        }
    }
};

// Global error manager instance
extern ErrorManager g_error_manager;

// === CONVENIENCE MACROS ===

#define REPORT_ERROR(code, component, details) \
    g_error_manager.reportError(code, component, details)

#define REPORT_MOTOR_TIMEOUT(motor, timeout) \
    g_error_manager.reportMotorTimeout(motor, timeout)

#define REPORT_INVALID_SPEED(component, speed, max_speed) \
    g_error_manager.reportInvalidSpeed(component, speed, max_speed)

#define REPORT_MOVEMENT_BLOCKED(component) \
    g_error_manager.reportMovementBlocked(component)

#define HAS_ERROR() g_error_manager.hasError()
#define HAS_CRITICAL_ERROR() g_error_manager.hasCriticalError()
#define CLEAR_ERROR() g_error_manager.clearError()
#define GET_ERROR_STRING() g_error_manager.getCurrentErrorString()

#endif // ERROR_SYSTEM_H