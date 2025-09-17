/*
  Path Segments Example
  
  Demonstrates low-level path segment control for precise robot movements.
  This example shows how to create custom path segments with different
  motor speeds and directions for complex movements.
  
  This approach gives you full control over each motor independently,
  allowing for precise curved paths and complex maneuvers.
*/

#include <RobotMotionControl.h>

// Create robot instance
RobotMotionControl robot;

void setup() {
  Serial.begin(9600);
  Serial.println("Robot Motion Control - Path Segments Example");
  
  // Initialize robot
  robot.begin();
  
  Serial.println("Robot initialized. Starting path segment demonstration...");
  delay(2000);
  
  // Demonstrate various path segments
  demonstratePathSegments();
}

void loop() {
  robot.update();
  delay(10);
}

void demonstratePathSegments() {
  // Example 1: Straight movement using path segments
  Serial.println("1. Straight movement using PathSegment");
  PathSegment straight(DIRECTION_FORWARD, 200, 500,    // Left: forward, 200 steps, 500 steps/sec
                      DIRECTION_FORWARD, 200, 500,     // Right: forward, 200 steps, 500 steps/sec
                      PEN_UP);                          // Pen up
  robot.executeSegment(straight);
  delay(1000);
  
  // Example 2: Sharp right turn
  Serial.println("2. Sharp right turn");
  PathSegment rightTurn(DIRECTION_FORWARD, 150, 400,   // Left: forward (outer wheel)
                       DIRECTION_BACKWARD, 150, 400,   // Right: backward (inner wheel)
                       PEN_UP);
  robot.executeSegment(rightTurn);
  delay(1000);
  
  // Example 3: Gentle curve with different speeds
  Serial.println("3. Gentle left curve with pen down");
  robot.penDown();
  PathSegment leftCurve(DIRECTION_FORWARD, 300, 300,   // Left: slower (inner wheel)
                       DIRECTION_FORWARD, 400, 500,    // Right: faster (outer wheel)
                       PEN_DOWN);
  robot.executeSegment(leftCurve);
  delay(1000);
  
  // Example 4: Complex S-curve using multiple segments
  Serial.println("4. S-curve using multiple segments");
  
  // First part of S - curve right
  PathSegment sCurve1(DIRECTION_FORWARD, 200, 600,     // Left: faster
                     DIRECTION_FORWARD, 150, 400,      // Right: slower
                     PEN_DOWN);
  robot.executeSegment(sCurve1);
  delay(500);
  
  // Second part of S - curve left
  PathSegment sCurve2(DIRECTION_FORWARD, 150, 400,     // Left: slower
                     DIRECTION_FORWARD, 200, 600,      // Right: faster
                     PEN_DOWN);
  robot.executeSegment(sCurve2);
  delay(1000);
  
  // Example 5: Spin in place
  Serial.println("5. Spinning in place");
  robot.penUp();
  PathSegment spin(DIRECTION_FORWARD, 300, 500,        // Left: forward
                  DIRECTION_BACKWARD, 300, 500,        // Right: backward
                  PEN_UP);
  robot.executeSegment(spin);
  delay(1000);
  
  // Example 6: Async segment execution
  Serial.println("6. Demonstrating async execution");
  PathSegment asyncSegment(DIRECTION_FORWARD, 400, 300,
                          DIRECTION_FORWARD, 400, 300,
                          PEN_UP);
  
  robot.executeSegmentAsync(asyncSegment);
  
  // Do other work while segment executes
  int counter = 0;
  while (robot.isBusy()) {
    Serial.print("Working... ");
    Serial.println(counter++);
    robot.update(); // Important: call update() in loop for async execution
    delay(100);
  }
  
  Serial.println("Path segments demonstration complete!");
  printRobotStatus();
}

void printRobotStatus() {
  Serial.println("\n--- Robot Status ---");
  Serial.print("Position X: ");
  Serial.print(robot.getX());
  Serial.println(" mm");
  
  Serial.print("Position Y: ");
  Serial.print(robot.getY());
  Serial.println(" mm");
  
  Serial.print("Angle: ");
  Serial.print(robot.getAngle() * 180.0 / PI);
  Serial.println(" degrees");
  
  Serial.print("Pen state: ");
  Serial.println(robot.isPenDown() ? "DOWN" : "UP");
  
  Serial.print("Busy: ");
  Serial.println(robot.isBusy() ? "YES" : "NO");
  Serial.println("-------------------\n");
}