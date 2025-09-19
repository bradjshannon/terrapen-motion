/**
 * Unit Tests for ServoDriver
 * 
 * Tests servo positioning, smooth movement, and state tracking.
 */

#include "../testing/TestFramework.h"
#include "../hardware/ServoDriver.h"

// === SERVO DRIVER INITIALIZATION TESTS ===

TEST_CASE_HARDWARE("ServoDriver_Initialization") {
    ServoDriver servo;
    
    // Test uninitialized state
    TEST_ASSERT_FALSE(servo.isInitialized());
    TEST_ASSERT_FALSE(servo.isAttached());
    
    // Test initialization
    servo.begin(9);  // Standard servo pin
    TEST_ASSERT_TRUE(servo.isInitialized());
    TEST_ASSERT_TRUE(servo.isAttached());
    
    return true;
}

TEST_CASE_LOGIC("ServoDriver_InitialPosition") {
    ServoDriver servo;
    servo.begin(9, 45);  // Initialize at 45 degrees
    
    TEST_ASSERT_EQUAL(45, servo.getCurrentAngle());
    TEST_ASSERT_EQUAL(45, servo.getTargetAngle());
    TEST_ASSERT_FALSE(servo.isMoving());
    
    return true;
}

// === POSITION CONTROL TESTS ===

TEST_CASE_HARDWARE("ServoDriver_ImmediatePositioning") {
    ServoDriver servo;
    servo.begin(9);
    
    // Test immediate angle setting
    servo.setAngle(90);
    TEST_ASSERT_EQUAL(90, servo.getCurrentAngle());
    TEST_ASSERT_EQUAL(90, servo.getTargetAngle());
    TEST_ASSERT_FALSE(servo.isMoving());
    
    servo.setAngle(180);
    TEST_ASSERT_EQUAL(180, servo.getCurrentAngle());
    TEST_ASSERT_FALSE(servo.isMoving());
    
    return true;
}

TEST_CASE_LOGIC("ServoDriver_AngleBounds") {
    ServoDriver servo;
    servo.begin(9);
    
    // Test angle clamping
    servo.setAngle(-10);  // Below minimum
    TEST_ASSERT_TRUE(servo.getCurrentAngle() >= 0);
    
    servo.setAngle(200);  // Above maximum
    TEST_ASSERT_TRUE(servo.getCurrentAngle() <= 180);
    
    // Test valid range
    servo.setAngle(45);
    TEST_ASSERT_EQUAL(45, servo.getCurrentAngle());
    
    servo.setAngle(135);
    TEST_ASSERT_EQUAL(135, servo.getCurrentAngle());
    
    return true;
}

// === SMOOTH MOVEMENT TESTS ===

TEST_CASE_HARDWARE("ServoDriver_SmoothMovement") {
    ServoDriver servo;
    servo.begin(9, 0);  // Start at 0 degrees
    
    // Start smooth movement
    servo.sweepTo(90, 1000);  // Move to 90° over 1 second
    
    // Should be moving
    TEST_ASSERT_TRUE(servo.isMoving());
    TEST_ASSERT_EQUAL(90, servo.getTargetAngle());
    TEST_ASSERT_EQUAL(0, servo.getCurrentAngle());  // Should still be at start
    
    // Progress should be near 0 initially
    TEST_ASSERT_TRUE(servo.getProgress() < 0.1);
    
    return true;
}

TEST_CASE_HARDWARE("ServoDriver_SmoothMovementProgress") {
    ServoDriver servo;
    servo.begin(9, 0);
    
    servo.sweepTo(180, 500);  // 500ms movement
    
    // Simulate time progression
    delay(100);  // 20% of movement time
    servo.update();
    
    float progress = servo.getProgress();
    TEST_ASSERT_TRUE(progress > 0.1 && progress < 0.4);  // Should be roughly 20%
    
    // Current angle should be between start and target
    int current = servo.getCurrentAngle();
    TEST_ASSERT_TRUE(current > 0 && current < 180);
    
    return true;
}

TEST_CASE_HARDWARE("ServoDriver_MovementCompletion") {
    ServoDriver servo;
    servo.begin(9, 0);
    
    servo.sweepTo(90, 100);  // Very short movement
    
    // Wait for completion
    unsigned long timeout = millis() + 200;
    while (servo.isMoving() && millis() < timeout) {
        servo.update();
        delay(10);
    }
    
    // Should be complete
    TEST_ASSERT_FALSE(servo.isMoving());
    TEST_ASSERT_EQUAL(90, servo.getCurrentAngle());
    TEST_ASSERT_NEAR(1.0, servo.getProgress(), 0.1);
    
    return true;
}

// === STATE MANAGEMENT TESTS ===

TEST_CASE_LOGIC("ServoDriver_MovementInterruption") {
    ServoDriver servo;
    servo.begin(9, 0);
    
    // Start movement
    servo.sweepTo(180, 1000);
    TEST_ASSERT_TRUE(servo.isMoving());
    
    // Interrupt with immediate positioning
    servo.setAngle(90);
    TEST_ASSERT_FALSE(servo.isMoving());
    TEST_ASSERT_EQUAL(90, servo.getCurrentAngle());
    TEST_ASSERT_EQUAL(90, servo.getTargetAngle());
    
    return true;
}

TEST_CASE_LOGIC("ServoDriver_MovementStop") {
    ServoDriver servo;
    servo.begin(9, 0);
    
    servo.sweepTo(180, 1000);
    TEST_ASSERT_TRUE(servo.isMoving());
    
    // Stop movement
    servo.stop();
    TEST_ASSERT_FALSE(servo.isMoving());
    
    // Position should be held at current location
    int stopped_angle = servo.getCurrentAngle();
    TEST_ASSERT_EQUAL(stopped_angle, servo.getTargetAngle());
    
    return true;
}

// === UPDATE FUNCTION TESTS ===

TEST_CASE_HARDWARE("ServoDriver_UpdateFrequency") {
    ServoDriver servo;
    servo.begin(9, 0);
    
    servo.sweepTo(90, 200);  // 200ms movement
    
    int update_count = 0;
    unsigned long start_time = millis();
    
    // Call update frequently and count how many times it actually changes position
    int last_angle = servo.getCurrentAngle();
    while (servo.isMoving() && millis() - start_time < 300) {
        servo.update();
        
        if (servo.getCurrentAngle() != last_angle) {
            update_count++;
            last_angle = servo.getCurrentAngle();
        }
        
        delay(5);  // 5ms between updates
    }
    
    // Should have multiple position updates during movement
    TEST_ASSERT_TRUE(update_count > 5);
    
    return true;
}

// === POWER MANAGEMENT TESTS ===

TEST_CASE_HARDWARE("ServoDriver_Detachment") {
    ServoDriver servo;
    servo.begin(9);
    
    TEST_ASSERT_TRUE(servo.isAttached());
    
    servo.detach();
    TEST_ASSERT_FALSE(servo.isAttached());
    TEST_ASSERT_FALSE(servo.isInitialized());
    
    return true;
}

// === ERROR CONDITION TESTS ===

TEST_CASE_LOGIC("ServoDriver_UninitializedSafety") {
    ServoDriver servo;
    
    // All operations should be safe on uninitialized servo
    TEST_ASSERT_FALSE(servo.isMoving());
    TEST_ASSERT_FALSE(servo.isAttached());
    
    // These should not crash
    servo.setAngle(90);
    servo.sweepTo(180, 500);
    servo.update();
    servo.stop();
    servo.detach();
    
    return true;
}

TEST_CASE_LOGIC("ServoDriver_ZeroDurationMovement") {
    ServoDriver servo;
    servo.begin(9, 0);
    
    // Zero duration should work like immediate positioning
    servo.sweepTo(90, 0);
    TEST_ASSERT_FALSE(servo.isMoving());
    TEST_ASSERT_EQUAL(90, servo.getCurrentAngle());
    
    return true;
}

TEST_CASE_LOGIC("ServoDriver_SameTargetMovement") {
    ServoDriver servo;
    servo.begin(9, 45);
    
    // Moving to current position should not start movement
    servo.sweepTo(45, 1000);
    TEST_ASSERT_FALSE(servo.isMoving());
    TEST_ASSERT_EQUAL(45, servo.getCurrentAngle());
    
    return true;
}