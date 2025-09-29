#ifdef INTEGRATION_TEST

#include <unity.h>
#include <Arduino.h>
#include <TerraPenRobot.h>

TerraPenRobot* test_robot = nullptr;

void setUp(void) {
    if (!test_robot) {
        test_robot = new TerraPenRobot();
    }
}

void tearDown(void) {
    if (test_robot) {
        test_robot->emergencyStop();
        delay(100);
    }
}

// Test actual stepper motor movement
void test_stepper_motor_steps(void) {
    TEST_ASSERT_NOT_NULL(test_robot);
    
    // Test basic step execution
    bool result = test_robot->stepLeft(10);
    TEST_ASSERT_TRUE(result);
    delay(500);  // Allow time for movement
    
    result = test_robot->stepRight(10);
    TEST_ASSERT_TRUE(result);
    delay(500);
}

// Test servo pen control with actual hardware
void test_servo_pen_hardware(void) {
    TEST_ASSERT_NOT_NULL(test_robot);
    
    // Test physical pen movement
    test_robot->penUp();
    delay(1000);  // Allow time for servo movement
    TEST_ASSERT_FALSE(test_robot->isPenDown());
    
    test_robot->penDown();
    delay(1000);  // Allow time for servo movement  
    TEST_ASSERT_TRUE(test_robot->isPenDown());
}

// Test coordinated movement with hardware
void test_coordinated_movement_hardware(void) {
    TEST_ASSERT_NOT_NULL(test_robot);
    
    Position start = test_robot->getCurrentPosition();
    
    // Execute a small movement
    bool result = test_robot->moveTo(start.x + 5.0, start.y + 5.0);
    TEST_ASSERT_TRUE(result);
    
    // Wait for movement to complete
    unsigned long timeout = millis() + 5000;  // 5 second timeout
    while (test_robot->getState() == TerraPenRobot::MOVING && millis() < timeout) {
        test_robot->update();
        delay(10);
    }
    
    TEST_ASSERT_EQUAL(TerraPenRobot::IDLE, test_robot->getState());
}

// Test emergency stop functionality
void test_emergency_stop_hardware(void) {
    TEST_ASSERT_NOT_NULL(test_robot);
    
    // Start a movement
    test_robot->moveTo(50.0, 50.0);
    delay(100);  // Let movement start
    
    // Emergency stop
    test_robot->emergencyStop();
    TEST_ASSERT_EQUAL(TerraPenRobot::EMERGENCY_STOP, test_robot->getState());
    
    // Recovery
    test_robot->clearError();
    TEST_ASSERT_EQUAL(TerraPenRobot::IDLE, test_robot->getState());
}

void setup() {
    Serial.begin(9600);
    delay(2000);  // Wait for serial to initialize
    
    UNITY_BEGIN();
    RUN_TEST(test_stepper_motor_steps);
    RUN_TEST(test_servo_pen_hardware);
    RUN_TEST(test_coordinated_movement_hardware);
    RUN_TEST(test_emergency_stop_hardware);
    UNITY_END();
}

void loop() {
    // Tests run once in setup()
}

#endif // INTEGRATION_TEST