/*
  Basic Movement Example
  
  Demonstrates basic robot movement functions including:
  - Straight line movement
  - Rotation
  - Pen control
  
  Hardware Requirements:
  - Arduino Nano
  - Two 28BYJ stepper motors
  - ULN2803A driver IC
  - Servo motor for pen control
  
  Wiring:
  - Left motor: pins 2,3,4,5 -> ULN2803A inputs
  - Right motor: pins 6,7,8,9 -> ULN2803A inputs  
  - Servo: pin 10
*/

#include <RobotMotionControl.h>

// Create robot instance with default configuration
RobotMotionControl robot;

void setup() {
  Serial.begin(9600);
  Serial.println("Robot Motion Control - Basic Movement Example");
  
  // Initialize robot with default settings
  robot.begin();
  
  Serial.println("Robot initialized. Starting movement sequence...");
  delay(2000);
  
  // Basic movement demonstration
  demonstrateBasicMovements();
}

void loop() {
  // Update robot for async operations (if using executeSegmentAsync)
  robot.update();
  
  // Add your main program here
  delay(10);
}

void demonstrateBasicMovements() {
  Serial.println("1. Moving forward 100mm");
  robot.moveStraight(100, 500, false); // 100mm forward, 500 steps/sec, pen up
  delay(1000);
  
  Serial.println("2. Rotating 90 degrees clockwise");
  robot.rotate(90, 300); // 90 degrees, 300 steps/sec
  delay(1000);
  
  Serial.println("3. Moving forward 50mm with pen down");
  robot.penDown();
  robot.moveStraight(50, 400, true); // 50mm forward, pen down
  delay(1000);
  
  Serial.println("4. Rotating 180 degrees counter-clockwise");
  robot.rotate(-180, 300);
  delay(1000);
  
  Serial.println("5. Moving back to start");
  robot.moveStraight(50, 400, true); // Continue drawing
  robot.penUp();
  delay(1000);
  
  Serial.println("6. Final rotation to original orientation");
  robot.rotate(90, 300);
  delay(1000);
  
  Serial.println("Movement sequence complete!");
  Serial.print("Final position - X: ");
  Serial.print(robot.getX());
  Serial.print("mm, Y: ");
  Serial.print(robot.getY());
  Serial.print("mm, Angle: ");
  Serial.print(robot.getAngle() * 180 / PI);
  Serial.println(" degrees");
}