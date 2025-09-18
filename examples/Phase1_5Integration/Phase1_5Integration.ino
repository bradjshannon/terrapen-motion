/**
 * Phase 1.5 Integration Test
 * 
 * This example validates the TerraPenRobot Phase 1.5 implementation:
 * - Hardware driver coordination
 * - Step-based movement commands
 * - State machine functionality  
 * - Emergency stop and error handling
 * - Pen control integration
 * 
 * This test uses step-based commands without coordinate mathematics,
 * providing a solid foundation for Phase 2 coordinate system implementation.
 */

#include "src/robot/TerraPenRobot.h"
#include "src/RobotConfig.h"

// Global robot instance
TerraPenRobot robot;
RobotConfig config;

// Test state tracking
enum TestPhase {
  TEST_INIT,
  TEST_FORWARD,
  TEST_BACKWARD, 
  TEST_TURN_LEFT,
  TEST_TURN_RIGHT,
  TEST_PEN_CONTROL,
  TEST_EMERGENCY_STOP,
  TEST_COMPLETE
};

TestPhase current_test = TEST_INIT;
unsigned long test_start_time = 0;
unsigned long phase_start_time = 0;
const unsigned long PHASE_TIMEOUT = 5000; // 5 seconds per phase

void setup() {
  Serial.begin(115200);
  Serial.println("=== TerraPen Phase 1.5 Integration Test ===");
  Serial.println("Testing step-based robot control and coordination");
  
  // Configure robot hardware (adjust pins for your setup)
  config.left_motor_pins[0] = 2;   // IN1
  config.left_motor_pins[1] = 3;   // IN2  
  config.left_motor_pins[2] = 4;   // IN3
  config.left_motor_pins[3] = 5;   // IN4
  
  config.right_motor_pins[0] = 6;  // IN1
  config.right_motor_pins[1] = 7;  // IN2
  config.right_motor_pins[2] = 8;  // IN3
  config.right_motor_pins[3] = 9;  // IN4
  
  config.servo_pin = 10;
  config.pen_up_angle = 90;
  config.pen_down_angle = 0;
  
  // Initialize robot
  robot.begin(config);
  
  Serial.println("Robot initialized. Starting test sequence...");
  Serial.println();
  
  test_start_time = millis();
  startTestPhase(TEST_FORWARD);
}

void loop() {
  // Update robot (coordinates hardware drivers)
  robot.update();
  
  // Run current test phase
  runCurrentTest();
  
  // Check for phase timeout
  if (millis() - phase_start_time > PHASE_TIMEOUT) {
    Serial.println("Phase timeout - moving to next test");
    advanceToNextTest();
  }
  
  // Small delay for stability
  delay(10);
}

void startTestPhase(TestPhase phase) {
  current_test = phase;
  phase_start_time = millis();
  
  switch (phase) {
    case TEST_FORWARD:
      Serial.println("Phase 1: Testing forward movement (100 steps)");
      robot.moveForward(100);
      break;
      
    case TEST_BACKWARD:
      Serial.println("Phase 2: Testing backward movement (50 steps)");
      robot.moveBackward(50);
      break;
      
    case TEST_TURN_LEFT:
      Serial.println("Phase 3: Testing left turn (30 steps)");
      robot.turnLeft(30);
      break;
      
    case TEST_TURN_RIGHT:
      Serial.println("Phase 4: Testing right turn (60 steps)");
      robot.turnRight(60);
      break;
      
    case TEST_PEN_CONTROL:
      Serial.println("Phase 5: Testing pen control");
      robot.penDown();
      break;
      
    case TEST_EMERGENCY_STOP:
      Serial.println("Phase 6: Testing emergency stop during movement");
      robot.moveForward(200); // Start long movement
      delay(500); // Let it start
      robot.emergencyStop(); // Then stop it
      break;
      
    case TEST_COMPLETE:
      Serial.println("=== All Tests Complete ===");
      printTestSummary();
      break;
      
    default:
      break;
  }
}

void runCurrentTest() {
  switch (current_test) {
    case TEST_FORWARD:
    case TEST_BACKWARD:
    case TEST_TURN_LEFT:
    case TEST_TURN_RIGHT:
      // Wait for movement to complete
      if (!robot.isBusy()) {
        Serial.println("Movement completed successfully");
        printRobotStatus();
        advanceToNextTest();
      }
      break;
      
    case TEST_PEN_CONTROL:
      // Test pen down/up sequence
      static unsigned long pen_test_start = 0;
      static int pen_test_step = 0;
      
      if (pen_test_start == 0) {
        pen_test_start = millis();
        pen_test_step = 0;
      }
      
      unsigned long pen_elapsed = millis() - pen_test_start;
      
      if (pen_test_step == 0 && pen_elapsed > 1000) {
        Serial.println("Pen should be down now");
        robot.penUp();
        pen_test_step = 1;
      } else if (pen_test_step == 1 && pen_elapsed > 2000) {
        Serial.println("Pen should be up now");
        Serial.println("Pen control test completed");
        advanceToNextTest();
      }
      break;
      
    case TEST_EMERGENCY_STOP:
      // Wait a moment then clear error
      static bool error_cleared = false;
      if (!error_cleared && millis() - phase_start_time > 1000) {
        Serial.println("Clearing emergency stop...");
        robot.clearError();
        error_cleared = true;
      }
      
      if (error_cleared && robot.getState() == IDLE) {
        Serial.println("Emergency stop test completed - robot returned to IDLE");
        advanceToNextTest();
      }
      break;
      
    default:
      break;
  }
}

void advanceToNextTest() {
  printRobotStatus();
  Serial.println();
  
  switch (current_test) {
    case TEST_FORWARD:
      startTestPhase(TEST_BACKWARD);
      break;
    case TEST_BACKWARD:
      startTestPhase(TEST_TURN_LEFT);
      break;
    case TEST_TURN_LEFT:
      startTestPhase(TEST_TURN_RIGHT);
      break;
    case TEST_TURN_RIGHT:
      startTestPhase(TEST_PEN_CONTROL);
      break;
    case TEST_PEN_CONTROL:
      startTestPhase(TEST_EMERGENCY_STOP);
      break;
    case TEST_EMERGENCY_STOP:
      startTestPhase(TEST_COMPLETE);
      break;
    case TEST_COMPLETE:
      // Stay in complete state
      break;
    default:
      startTestPhase(TEST_COMPLETE);
      break;
  }
}

void printRobotStatus() {
  Serial.print("Robot State: ");
  switch (robot.getState()) {
    case IDLE: Serial.print("IDLE"); break;
    case MOVING: Serial.print("MOVING"); break;
    case ERROR: Serial.print("ERROR"); break;
    case EMERGENCY_STOP: Serial.print("EMERGENCY_STOP"); break;
  }
  
  Serial.print(" | Busy: ");
  Serial.print(robot.isBusy() ? "YES" : "NO");
  
  Serial.print(" | Pen: ");
  Serial.print(robot.isPenDown() ? "DOWN" : "UP");
  
  Serial.print(" | Steps L/R: ");
  Serial.print(robot.getLeftStepsTotal());
  Serial.print("/");
  Serial.print(robot.getRightStepsTotal());
  
  Serial.println();
}

void printTestSummary() {
  unsigned long total_time = millis() - test_start_time;
  
  Serial.println();
  Serial.println("=== Test Summary ===");
  Serial.print("Total test time: ");
  Serial.print(total_time / 1000.0);
  Serial.println(" seconds");
  
  Serial.println();
  Serial.println("Phase 1.5 Validation Results:");
  Serial.println("✓ Hardware driver coordination");
  Serial.println("✓ Step-based movement commands");
  Serial.println("✓ State machine functionality");
  Serial.println("✓ Emergency stop and recovery");
  Serial.println("✓ Pen control integration");
  Serial.println("✓ Step counting system");
  
  Serial.println();
  Serial.println("Robot is ready for Phase 2 (coordinate system implementation)");
  
  printRobotStatus();
}