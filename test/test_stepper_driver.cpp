/**
 * Unit Tests for StepperDriver
 * 
 * Tests timing accuracy, state management, and hardware coordination.
 * These tests validate the core functionality that Phase 2 depends on.
 */

#include "../testing/TestFramework.h"
#include "../hardware/StepperDriver.h"

// === STEPPER DRIVER INITIALIZATION TESTS ===

TEST_CASE_HARDWARE("StepperDriver_Initialization") {
    StepperDriver motor;
    
    // Test uninitialized state
    TEST_ASSERT_FALSE(motor.isInitialized());
    TEST_ASSERT_FALSE(motor.isReady());
    
    // Test initialization
    motor.begin(2, 3, 4, 5);
    TEST_ASSERT_TRUE(motor.isInitialized());
    TEST_ASSERT_TRUE(motor.isReady());
    
    return true;
}

TEST_CASE_LOGIC("StepperDriver_SpeedConfiguration") {
    StepperDriver motor;
    motor.begin(2, 3, 4, 5);
    
    // Test speed setting and retrieval
    motor.setSpeed(100.0);
    TEST_ASSERT_EQUAL(100.0, motor.getSpeed());
    
    motor.setSpeed(250.5);
    TEST_ASSERT_EQUAL(250.5, motor.getSpeed());
    
    // Test speed limits (typical 28BYJ-48 limits)
    motor.setSpeed(1000.0);  // Very high speed
    TEST_ASSERT_TRUE(motor.getSpeed() <= 500.0);  // Should be clamped
    
    return true;
}

// === TIMING AND COORDINATION TESTS ===

TEST_CASE_HARDWARE("StepperDriver_TimingAccuracy") {
    StepperDriver motor;
    motor.begin(2, 3, 4, 5);
    motor.setSpeed(100.0);  // 100 steps/sec = 10ms intervals
    
    // First step should work immediately
    unsigned long start_time = micros();
    bool first_step = motor.stepForward();
    unsigned long first_duration = micros() - start_time;
    
    TEST_ASSERT_TRUE(first_step);
    TEST_ASSERT_TRUE(first_duration < 1000);  // Should be very fast (<1ms)
    
    // Second step should be blocked
    TEST_ASSERT_FALSE(motor.stepForward());
    
    // After waiting, should work again
    delay(15);  // Wait longer than 10ms interval
    TEST_ASSERT_TRUE(motor.stepForward());
    
    return true;
}

TEST_CASE_HARDWARE("StepperDriver_NonBlockingOperation") {
    StepperDriver motor;
    motor.begin(2, 3, 4, 5);
    motor.setSpeed(50.0);  // 50 steps/sec = 20ms intervals
    
    // Take one step
    TEST_ASSERT_TRUE(motor.stepForward());
    
    // Verify non-blocking behavior during wait period
    unsigned long start = millis();
    while (millis() - start < 15) {  // Wait 15ms of 20ms interval
        TEST_ASSERT_FALSE(motor.stepForward());  // Should consistently return false
        // This loop should execute many times without blocking
    }
    
    // After full interval, should work
    delay(10);  // Complete the 20ms
    TEST_ASSERT_TRUE(motor.stepForward());
    
    return true;
}

// === DIRECTION AND PHASE TESTS ===

TEST_CASE_LOGIC("StepperDriver_DirectionControl") {
    StepperDriver motor;
    motor.begin(2, 3, 4, 5);
    
    int initial_phase = motor.getCurrentPhase();
    
    // Step forward and check phase advancement
    motor.stepNow(1);  // Use blocking step for deterministic testing
    int forward_phase = motor.getCurrentPhase();
    TEST_ASSERT_TRUE(forward_phase != initial_phase);
    
    // Step backward and check phase retreat
    motor.stepNow(-1);
    int backward_phase = motor.getCurrentPhase();
    TEST_ASSERT_EQUAL(initial_phase, backward_phase);
    
    return true;
}

TEST_CASE_LOGIC("StepperDriver_PhaseSequence") {
    StepperDriver motor;
    motor.begin(2, 3, 4, 5);
    
    // Test complete 8-phase cycle
    int initial_phase = motor.getCurrentPhase();
    
    for (int i = 0; i < 8; i++) {
        motor.stepNow(1);
    }
    
    // After 8 steps, should return to initial phase
    TEST_ASSERT_EQUAL(initial_phase, motor.getCurrentPhase());
    
    return true;
}

// === POWER MANAGEMENT TESTS ===

TEST_CASE_HARDWARE("StepperDriver_PowerManagement") {
    StepperDriver motor;
    motor.begin(2, 3, 4, 5);
    
    // Test hold state
    motor.hold();
    TEST_ASSERT_TRUE(motor.isHolding());
    
    // Test release state
    motor.release();
    TEST_ASSERT_FALSE(motor.isHolding());
    
    // Test that stepping automatically enables holding
    motor.stepNow(1);
    TEST_ASSERT_TRUE(motor.isHolding());
    
    return true;
}

// === STRESS TESTS ===

TEST_CASE_HARDWARE("StepperDriver_HighFrequencySteps") {
    StepperDriver motor;
    motor.begin(2, 3, 4, 5);
    motor.setSpeed(200.0);  // High speed
    
    int successful_steps = 0;
    unsigned long start_time = millis();
    
    // Attempt many steps over 100ms
    while (millis() - start_time < 100) {
        if (motor.stepForward()) {
            successful_steps++;
        }
    }
    
    // Should get approximately 20 steps (200 steps/sec * 0.1 sec)
    // Allow ±5 steps tolerance for timing variations
    TEST_ASSERT_TRUE(successful_steps >= 15 && successful_steps <= 25);
    
    return true;
}

// === ERROR CONDITION TESTS ===

TEST_CASE_LOGIC("StepperDriver_UninitializedSafety") {
    StepperDriver motor;
    
    // All operations should be safe on uninitialized motor
    TEST_ASSERT_FALSE(motor.stepForward());
    TEST_ASSERT_FALSE(motor.stepBackward());
    TEST_ASSERT_FALSE(motor.isReady());
    TEST_ASSERT_FALSE(motor.isHolding());
    
    // These should not crash
    motor.hold();
    motor.release();
    motor.setSpeed(100.0);
    
    return true;
}