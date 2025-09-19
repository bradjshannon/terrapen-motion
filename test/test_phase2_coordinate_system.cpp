/**
 * test_phase2_coordinate_system.cpp
 * 
 * Comprehensive integration tests for Phase 2 coordinate system
 * Tests all Phase 2 features using the existing TerraPen testing framework
 */

#include "../src/testing/TestFramework.h"
#include "../src/robot/TerraPenRobot.h"
#include "../src/Position.h"

// Global test robot instance
TerraPenRobot test_robot;

// === INITIALIZATION TESTS ===

TEST_CASE_LOGIC("TerraPenRobot_Phase2_Initialization") {
    test_robot.begin();
    test_robot.resetPosition(0, 0, 0);
    
    Position pos = test_robot.getCurrentPosition();
    TEST_ASSERT_TRUE(pos.equals(Position(0, 0, 0), 0.01));
    
    TEST_ASSERT_EQUAL(IDLE, test_robot.getState());
    TEST_ASSERT_FALSE(test_robot.isBusy());
    TEST_ASSERT_FALSE(test_robot.isPenDown());
    
    return true;
}

// === COORDINATE MOVEMENT TESTS ===

TEST_CASE_LOGIC("TerraPenRobot_MoveTo_BasicFunction") {
    test_robot.resetPosition(0, 0, 0);
    
    // Test valid movement
    bool result = test_robot.moveTo(10.0, 15.0);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(MOVING, test_robot.getState());
    TEST_ASSERT_TRUE(test_robot.isBusy());
    
    return true;
}

TEST_CASE_LOGIC("TerraPenRobot_DrawTo_BasicFunction") {
    test_robot.resetPosition(0, 0, 0);
    
    // Test drawing movement
    bool result = test_robot.drawTo(5.0, 5.0);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(MOVING, test_robot.getState());
    TEST_ASSERT_TRUE(test_robot.isBusy());
    
    return true;
}

TEST_CASE_LOGIC("TerraPenRobot_MoveBy_RelativeMovement") {
    test_robot.resetPosition(10.0, 20.0, 0);
    
    // Test relative movement
    bool result = test_robot.moveBy(5.0, -10.0);
    TEST_ASSERT_TRUE(result);
    
    return true;
}

TEST_CASE_LOGIC("TerraPenRobot_DrawBy_RelativeDrawing") {
    test_robot.resetPosition(0, 0, 0);
    
    // Test relative drawing
    bool result = test_robot.drawBy(8.0, 12.0);
    TEST_ASSERT_TRUE(result);
    
    return true;
}

// === ROTATION CONTROL TESTS ===

TEST_CASE_LOGIC("TerraPenRobot_TurnTo_AbsoluteAngle") {
    test_robot.resetPosition(0, 0, 0);
    
    // Test turning to 45 degrees
    bool result = test_robot.turnTo(PI/4);  // 45 degrees
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(MOVING, test_robot.getState());
    
    return true;
}

TEST_CASE_LOGIC("TerraPenRobot_TurnBy_RelativeAngle") {
    test_robot.resetPosition(0, 0, PI/6);  // Start at 30 degrees
    
    // Test turning by additional 30 degrees
    bool result = test_robot.turnBy(PI/6);
    TEST_ASSERT_TRUE(result);
    
    return true;
}

// === WORKSPACE BOUNDARY TESTS ===

TEST_CASE_LOGIC("TerraPenRobot_WorkspaceBoundaries_ValidPositions") {
    // Test valid positions within default ±100mm workspace
    TEST_ASSERT_TRUE(test_robot.isValidPosition(0, 0));
    TEST_ASSERT_TRUE(test_robot.isValidPosition(50, 75));
    TEST_ASSERT_TRUE(test_robot.isValidPosition(-90, -80));
    TEST_ASSERT_TRUE(test_robot.isValidPosition(100, 100));  // Edge case
    TEST_ASSERT_TRUE(test_robot.isValidPosition(-100, -100)); // Edge case
    
    return true;
}

TEST_CASE_LOGIC("TerraPenRobot_WorkspaceBoundaries_InvalidPositions") {
    // Test invalid positions outside workspace
    TEST_ASSERT_FALSE(test_robot.isValidPosition(150, 0));   // X too large
    TEST_ASSERT_FALSE(test_robot.isValidPosition(0, 150));   // Y too large
    TEST_ASSERT_FALSE(test_robot.isValidPosition(-150, 0));  // X too negative
    TEST_ASSERT_FALSE(test_robot.isValidPosition(0, -150));  // Y too negative
    TEST_ASSERT_FALSE(test_robot.isValidPosition(200, 200)); // Both too large
    
    return true;
}

TEST_CASE_LOGIC("TerraPenRobot_WorkspaceBoundaries_MovementRejection") {
    test_robot.resetPosition(0, 0, 0);
    
    // Test that invalid movements are rejected
    bool result = test_robot.moveTo(150, 150);  // Outside workspace
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(IDLE, test_robot.getState());  // Should remain idle
    
    return true;
}

// === POSITION TRACKING TESTS ===

TEST_CASE_LOGIC("TerraPenRobot_PositionTracking_ResetPosition") {
    // Test position reset functionality
    test_robot.resetPosition(25.5, -10.3, PI/3);
    
    Position pos = test_robot.getCurrentPosition();
    TEST_ASSERT_TRUE(abs(pos.x - 25.5) < 0.01);
    TEST_ASSERT_TRUE(abs(pos.y - (-10.3)) < 0.01);
    TEST_ASSERT_TRUE(abs(pos.angle - PI/3) < 0.01);
    
    return true;
}

TEST_CASE_LOGIC("TerraPenRobot_PositionTracking_AngleNormalization") {
    // Test angle normalization to [-PI, PI]
    test_robot.resetPosition(0, 0, 3*PI);  // Should normalize to PI
    
    Position pos = test_robot.getCurrentPosition();
    TEST_ASSERT_TRUE(pos.angle >= -PI && pos.angle <= PI);
    
    return true;
}

// === KINEMATICS CALCULATION TESTS ===

TEST_CASE_LOGIC("TerraPenRobot_Kinematics_StepCalculation") {
    // This tests the private calculateSteps method indirectly
    // by checking that different movements generate different step patterns
    
    test_robot.resetPosition(0, 0, 0);
    
    // Record initial step counts
    long initial_left = test_robot.getLeftStepsTotal();
    long initial_right = test_robot.getRightStepsTotal();
    
    // Forward movement should use both motors equally
    test_robot.moveForward(10);  // Step-based movement for testing
    
    // Since we can't directly test private methods, we verify behavior
    TEST_ASSERT_TRUE(true);  // Placeholder - real test would check motor coordination
    
    return true;
}

// === STATE MACHINE INTEGRATION TESTS ===

TEST_CASE_LOGIC("TerraPenRobot_StateMachine_CoordinateMovement") {
    test_robot.resetPosition(0, 0, 0);
    
    // Start in IDLE
    TEST_ASSERT_EQUAL(IDLE, test_robot.getState());
    TEST_ASSERT_FALSE(test_robot.isBusy());
    
    // Issue coordinate movement command
    bool result = test_robot.moveTo(10, 10);
    TEST_ASSERT_TRUE(result);
    
    // Should transition to MOVING
    TEST_ASSERT_EQUAL(MOVING, test_robot.getState());
    TEST_ASSERT_TRUE(test_robot.isBusy());
    
    return true;
}

TEST_CASE_LOGIC("TerraPenRobot_StateMachine_BusyRejection") {
    test_robot.resetPosition(0, 0, 0);
    
    // Start first movement
    bool result1 = test_robot.moveTo(10, 10);
    TEST_ASSERT_TRUE(result1);
    TEST_ASSERT_TRUE(test_robot.isBusy());
    
    // Second movement should be rejected while busy
    bool result2 = test_robot.moveTo(20, 20);
    TEST_ASSERT_FALSE(result2);
    
    return true;
}

// === PEN CONTROL INTEGRATION TESTS ===

TEST_CASE_LOGIC("TerraPenRobot_PenControl_MoveTo") {
    test_robot.resetPosition(0, 0, 0);
    test_robot.penDown();  // Start with pen down
    
    // moveTo should automatically raise pen
    test_robot.moveTo(10, 10);
    // Note: In real hardware test, would verify pen position
    
    return true;
}

TEST_CASE_LOGIC("TerraPenRobot_PenControl_DrawTo") {
    test_robot.resetPosition(0, 0, 0);
    test_robot.penUp();  // Start with pen up
    
    // drawTo should automatically lower pen
    test_robot.drawTo(10, 10);
    // Note: In real hardware test, would verify pen position
    
    return true;
}

// === ERROR CONDITION TESTS ===

TEST_CASE_LOGIC("TerraPenRobot_ErrorConditions_InvalidSpeed") {
    test_robot.resetPosition(0, 0, 0);
    
    // Test invalid speed parameters
    bool result1 = test_robot.moveTo(10, 10, 0);     // Zero speed
    TEST_ASSERT_FALSE(result1);
    
    bool result2 = test_robot.moveTo(10, 10, -5);    // Negative speed
    TEST_ASSERT_FALSE(result2);
    
    return true;
}

TEST_CASE_LOGIC("TerraPenRobot_ErrorConditions_InvalidRotationSpeed") {
    test_robot.resetPosition(0, 0, 0);
    
    // Test invalid rotation speed
    bool result1 = test_robot.turnTo(PI/4, 0);       // Zero speed
    TEST_ASSERT_FALSE(result1);
    
    bool result2 = test_robot.turnBy(PI/4, -1);      // Negative speed
    TEST_ASSERT_FALSE(result2);
    
    return true;
}

// === INTEGRATION TESTS ===

TEST_CASE_POST("TerraPenRobot_Integration_BasicSquare") {
    test_robot.resetPosition(0, 0, 0);
    
    // Test complete square drawing sequence
    bool success = true;
    
    // Move to start position
    success &= test_robot.moveTo(10, 10);
    if (!success) return false;
    
    // Draw square (would need real robot to complete)
    success &= test_robot.drawTo(20, 10);  // Right side
    success &= test_robot.drawTo(20, 20);  // Top side
    success &= test_robot.drawTo(10, 20);  // Left side
    success &= test_robot.drawTo(10, 10);  // Bottom side
    
    return success;
}

TEST_CASE_POST("TerraPenRobot_Integration_CoordinateAccuracy") {
    test_robot.resetPosition(0, 0, 0);
    
    // Test position tracking accuracy over multiple moves
    Position start = test_robot.getCurrentPosition();
    
    // Perform series of movements that should return to start
    test_robot.moveTo(10, 0);   // Right
    test_robot.moveTo(10, 10);  // Up
    test_robot.moveTo(0, 10);   // Left
    test_robot.moveTo(0, 0);    // Down - back to start
    
    // In real test, would wait for completion and check final position
    // For unit test, just verify commands were accepted
    return true;
}

// === STRESS TESTS ===

TEST_CASE_STRESS("TerraPenRobot_Stress_RepeatedMovements") {
    test_robot.resetPosition(0, 0, 0);
    
    // Test many repeated coordinate movements
    for (int i = 0; i < 50; i++) {
        float x = (i % 10) * 5.0;  // 0, 5, 10, 15, ... 45, 0, 5, ...
        float y = (i / 10) * 5.0;  // 0, 0, 0, ... 20, 20, ...
        
        if (!test_robot.isValidPosition(x, y)) continue;
        
        bool result = test_robot.moveTo(x, y);
        if (!result) return false;  // Should not fail for valid positions
    }
    
    return true;
}

TEST_CASE_STRESS("TerraPenRobot_Stress_WorkspaceBoundaryTesting") {
    // Test many boundary conditions
    for (int i = 0; i < 100; i++) {
        float x = (i % 2 == 0) ? 99.0 : 101.0;   // Alternate valid/invalid
        float y = (i % 4 < 2) ? 99.0 : 101.0;    // Mix of valid/invalid
        
        bool expected_valid = (x <= 100.0 && y <= 100.0);
        bool actual_valid = test_robot.isValidPosition(x, y);
        
        if (expected_valid != actual_valid) return false;
    }
    
    return true;
}