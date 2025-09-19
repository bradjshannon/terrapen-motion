/**
 * Simple Coordinate Test - Phase 2 Basic Example
 * 
 * This example demonstrates the basic coordinate-based movement system
 * introduced in Phase 2. It shows how to use moveTo() and drawTo() 
 * commands to create precise drawings using 2D coordinates.
 * 
 * Hardware Requirements:
 * - Arduino Nano
 * - Two 28BYJ-48 stepper motors  
 * - ULN2803A driver IC
 * - Servo motor for pen control
 * 
 * Expected Behavior:
 * - Robot draws a 20mm x 20mm square
 * - Moves to new location and draws a triangle
 * - Demonstrates automatic pen up/down control
 * - Shows position tracking and coordinate system
 */

#include <TerraPenMotionControl.h>

TerraPenRobot robot;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("\n=== Simple Coordinate Test ===");
  Serial.println("Phase 2 coordinate-based movement demonstration");
  
  // Initialize robot with default configuration
  robot.begin();
  
  // Set starting position (coordinate system origin)
  robot.resetPosition(0, 0, 0);  // x=0, y=0, angle=0
  
  Serial.println("Robot initialized successfully!");
  Serial.println("Drawing patterns using coordinate commands...");
  Serial.println();
  
  delay(2000);
}

void loop() {
  // Always call update() to coordinate hardware
  robot.update();
  
  // Execute movements when robot is not busy
  if (!robot.isBusy()) {
    
    // === PATTERN 1: 20mm x 20mm SQUARE ===
    Serial.println("Drawing 20mm x 20mm square...");
    
    robot.moveTo(0, 0);       // Move to start position (pen up)
    robot.drawTo(20, 0);      // Draw right side (pen down)
    robot.drawTo(20, 20);     // Draw top side
    robot.drawTo(0, 20);      // Draw left side  
    robot.drawTo(0, 0);       // Complete square
    
    Serial.println("Square complete!");
    
    // === PATTERN 2: EQUILATERAL TRIANGLE ===
    Serial.println("Moving to new location and drawing triangle...");
    
    robot.moveTo(30, 30);     // Move to new position (pen up)
    robot.drawTo(45, 30);     // Base of triangle (pen down)
    robot.drawTo(37.5, 43);   // Peak of triangle (15mm height)
    robot.drawTo(30, 30);     // Close triangle
    
    Serial.println("Triangle complete!");
    
    // === PATTERN 3: SIMPLE LINE PATTERN ===
    Serial.println("Drawing line pattern...");
    
    robot.moveTo(0, 50);      // Move above square
    robot.drawTo(20, 50);     // Horizontal line
    robot.moveTo(10, 60);     // Move up
    robot.drawTo(10, 40);     // Vertical line (crosses horizontal)
    
    Serial.println("Line pattern complete!");
    
    // === POSITION REPORTING ===
    Position current_pos = robot.getCurrentPosition();
    Serial.print("\nFinal position: ");
    current_pos.print();  // Prints: "Position: (x, y) @ angle°"
    
    Serial.println("\n=== Pattern sequence complete! ===");
    Serial.println("Waiting 5 seconds before repeating...\n");
    
    delay(5000);  // Pause before repeating the entire sequence
  }
}

/**
 * This example demonstrates:
 * 
 * 1. Coordinate System Usage:
 *    - moveTo(x, y) - Move to position with pen up
 *    - drawTo(x, y) - Move to position with pen down
 *    - resetPosition() - Set coordinate system origin
 * 
 * 2. Automatic Pen Control:
 *    - moveTo() automatically raises pen before moving
 *    - drawTo() automatically lowers pen before drawing
 * 
 * 3. Position Tracking:
 *    - getCurrentPosition() returns estimated position
 *    - Based on differential drive kinematics
 * 
 * 4. Coordinate Units:
 *    - All coordinates are in millimeters
 *    - Origin (0,0) is set by resetPosition()
 *    - Positive X is right, positive Y is forward
 * 
 * 5. Non-blocking Operation:
 *    - Commands are queued and executed gradually
 *    - Always call robot.update() in loop()
 *    - Check robot.isBusy() before issuing new commands
 */