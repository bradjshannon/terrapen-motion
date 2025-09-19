/**
 * Unit Tests for TerraPenRobot
 * 
 * Tests robot coordination, state machine, and movement coordination.
 * These are integration tests that validate the robot-level functionality.
 */

#include "../testing/TestFramework.h"
#include "../robot/TerraPenRobot.h"
#include "../TerraPenConfig.h"

// === ROBOT INITIALIZATION TESTS ===

TEST_CASE_HARDWARE("TerraPenRobot_Initialization") {
    TerraPenRobot robot;
    
    // Should initialize successfully
    robot.begin();
    
    // Should start in IDLE state
    TEST_ASSERT_EQUAL(IDLE, robot.getState());
    
    // Should not be busy initially
    TEST_ASSERT_FALSE(robot.isBusy());
    
    // Pen should start in up position
    TEST_ASSERT_FALSE(robot.isPenDown());
    
    return true;
}

// === MOVEMENT TESTS ===

TEST_CASE_HARDWARE("TerraPenRobot_ForwardMovement") {
    TerraPenRobot robot;
    robot.begin();
    
    // Should accept forward movement command
    TEST_ASSERT_TRUE(robot.moveForward(10));
    
    // Should be in MOVING state
    TEST_ASSERT_EQUAL(MOVING, robot.getState());
    TEST_ASSERT_TRUE(robot.isBusy());
    
    return true;
}

TEST_CASE_HARDWARE("TerraPenRobot_BackwardMovement") {
    TerraPenRobot robot;
    robot.begin();
    
    // Should accept backward movement command
    TEST_ASSERT_TRUE(robot.moveBackward(10));
    
    // Should be in MOVING state
    TEST_ASSERT_EQUAL(MOVING, robot.getState());
    TEST_ASSERT_TRUE(robot.isBusy());
    
    return true;
}

TEST_CASE_HARDWARE("TerraPenRobot_LeftTurn") {
    TerraPenRobot robot;
    robot.begin();
    
    // Should accept left turn command
    TEST_ASSERT_TRUE(robot.turnLeft(5));
    
    // Should be in MOVING state
    TEST_ASSERT_EQUAL(MOVING, robot.getState());
    TEST_ASSERT_TRUE(robot.isBusy());
    
    return true;
}

TEST_CASE_HARDWARE("TerraPenRobot_RightTurn") {
    TerraPenRobot robot;
    robot.begin();
    
    // Should accept right turn command
    TEST_ASSERT_TRUE(robot.turnRight(5));
    
    // Should be in MOVING state
    TEST_ASSERT_EQUAL(MOVING, robot.getState());
    TEST_ASSERT_TRUE(robot.isBusy());
    
    return true;
}

// === PEN CONTROL TESTS ===

TEST_CASE_HARDWARE("TerraPenRobot_PenControl") {
    TerraPenRobot robot;
    robot.begin();
    
    // Initially pen should be up
    TEST_ASSERT_FALSE(robot.isPenDown());
    
    // Lower pen
    robot.penDown();
    TEST_ASSERT_TRUE(robot.isPenDown());
    
    // Raise pen
    robot.penUp();
    TEST_ASSERT_FALSE(robot.isPenDown());
    
    return true;
}

// === STATE MANAGEMENT TESTS ===

TEST_CASE_HARDWARE("TerraPenRobot_EmergencyStop") {
    TerraPenRobot robot;
    robot.begin();
    
    // Start a movement
    robot.moveForward(100);
    delay(1);  // Let it start
    
    // Emergency stop should work immediately
    robot.emergencyStop();
    TEST_ASSERT_EQUAL(EMERGENCY_STOP, robot.getState());
    
    return true;
}

TEST_CASE_HARDWARE("TerraPenRobot_BusyRejection") {
    TerraPenRobot robot;
    robot.begin();
    
    // Start first movement
    TEST_ASSERT_TRUE(robot.moveForward(50));
    
    // Second movement should be rejected while busy
    TEST_ASSERT_FALSE(robot.moveBackward(10));
    TEST_ASSERT_FALSE(robot.turnLeft(5));
    TEST_ASSERT_FALSE(robot.turnRight(5));
    
    return true;
}

// === ERROR CONDITION TESTS ===

TEST_CASE_LOGIC("TerraPenRobot_InvalidMovements") {
    TerraPenRobot robot;
    robot.begin();
    
    // Zero steps should be rejected
    TEST_ASSERT_FALSE(robot.moveForward(0));
    TEST_ASSERT_FALSE(robot.turnLeft(-5));  // Negative steps
    
    // Robot should remain in IDLE state
    TEST_ASSERT_EQUAL(IDLE, robot.getState());
    
    return true;
}

// === STEP TRACKING TESTS ===

TEST_CASE_HARDWARE("TerraPenRobot_StepCounting") {
    TerraPenRobot robot;
    robot.begin();
    
    // Initial step counts should be zero
    TEST_ASSERT_EQUAL(0, robot.getLeftStepsTotal());
    TEST_ASSERT_EQUAL(0, robot.getRightStepsTotal());
    
    // Move forward (both motors should increment equally)
    robot.moveForward(10);
    
    // Wait for movement to complete
    int timeout = 100;
    while (robot.isBusy() && timeout-- > 0) {
        robot.update();
        delay(10);
    }
    
    // Both motors should have moved the same amount
    TEST_ASSERT_EQUAL(robot.getLeftStepsTotal(), robot.getRightStepsTotal());
    TEST_ASSERT_TRUE(robot.getLeftStepsTotal() > 0);
    
    return true;
}

TEST_CASE_HARDWARE("TerraPenRobot_TurnStepCounting") {
    TerraPenRobot robot;
    robot.begin();
    
    // Reset counters (if there's a method for it, otherwise skip this test)
    // This test validates that turns affect step counts differently
    
    long initial_left = robot.getLeftStepsTotal();
    long initial_right = robot.getRightStepsTotal();
    
    // Turn left (right motor should move more)
    robot.turnLeft(5);
    
    // Wait for movement to complete
    int timeout = 100;
    while (robot.isBusy() && timeout-- > 0) {
        robot.update();
        delay(10);
    }
    
    // Step counts should have changed
    TEST_ASSERT_TRUE(robot.getLeftStepsTotal() != initial_left || 
                    robot.getRightStepsTotal() != initial_right);
    
    return true;
}