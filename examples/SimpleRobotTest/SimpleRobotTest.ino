/**
 * Simple Robot Test - Phase 1.5 Validation
 * 
 * Minimal test example to validate TerraPenRobot basic functionality.
 * This is a simpler version of the comprehensive integration test.
 */

#include <TerraPenMotionControl.h>

TerraPenRobot robot;

void setup() {
  Serial.begin(115200);
  Serial.println("TerraPen Robot - Simple Test");
  
  // Use default configuration
  robot.begin(config);
  
  Serial.println("Robot initialized successfully!");
  Serial.print("Initial state: ");
  printState();
}

void loop() {
  // Update robot hardware coordination
  robot.update();
  
  // Simple test sequence
  static unsigned long last_command = 0;
  static int test_step = 0;
  
  if (millis() - last_command > 3000 && !robot.isBusy()) {
    switch (test_step) {
      case 0:
        Serial.println("Moving forward 25 steps...");
        robot.moveForward(25);
        break;
        
      case 1:
        Serial.println("Moving backward 10 steps...");
        robot.moveBackward(10);
        break;
        
      case 2:
        Serial.println("Turning left 15 steps...");
        robot.turnLeft(15);
        break;
        
      case 3:
        Serial.println("Testing pen down...");
        robot.penDown();
        break;
        
      case 4:
        Serial.println("Testing pen up...");
        robot.penUp();
        break;
        
      default:
        Serial.println("Test sequence complete. Robot steps:");
        Serial.print("Left total: "); Serial.println(robot.getLeftStepsTotal());
        Serial.print("Right total: "); Serial.println(robot.getRightStepsTotal());
        Serial.println("Restarting sequence...");
        test_step = -1; // Will increment to 0
        break;
    }
    
    test_step++;
    last_command = millis();
    printState();
  }
}

void printState() {
  Serial.print("State: ");
  switch (robot.getState()) {
    case IDLE: Serial.print("IDLE"); break;
    case MOVING: Serial.print("MOVING"); break;
    case ERROR: Serial.print("ERROR"); break;
    case EMERGENCY_STOP: Serial.print("EMERGENCY_STOP"); break;
  }
  
  Serial.print(" | Pen: ");
  Serial.print(robot.isPenDown() ? "DOWN" : "UP");
  Serial.println();
}