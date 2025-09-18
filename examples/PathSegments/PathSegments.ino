/*
 * Path Segments Example
 * 
 * Demonstrates coordinated motor control to create different movement patterns.
 * This example shows how to use the hardware drivers to create complex robot
 * movements by coordinating stepper motors and servo control.
 * 
 * Movement Patterns Demonstrated:
 * - Straight movement (synchronized motors)
 * - Curves (different motor speeds)  
 * - Spins (motors in opposite directions)
 * - Complex patterns combining multiple movements
 * 
 * This approach gives you full control over each motor independently,
 * allowing for precise path control and complex maneuvers.
 */

#include <TerraPenMotionControl.h>

// Hardware drivers
StepperDriver left_motor;
StepperDriver right_motor;
ServoDriver pen_servo;

// Configuration
RobotConfig config;

// Position tracking
Position robot_position(0, 0, 0);

void setup() {
  Serial.begin(115200);
  Serial.println("=== Path Segments Example ===");
  Serial.println("Demonstrates coordinated motor control for complex movements");
  Serial.println();
  
  // Initialize hardware
  left_motor.begin(
    config.left_motor_pins[0], config.left_motor_pins[1],
    config.left_motor_pins[2], config.left_motor_pins[3]
  );
  
  right_motor.begin(
    config.right_motor_pins[0], config.right_motor_pins[1],
    config.right_motor_pins[2], config.right_motor_pins[3]
  );
  
  pen_servo.begin(config.servo_pin, config.pen_up_angle);
  
  Serial.println("Hardware initialized. Starting path demonstrations...");
  delay(2000);
  
  // Demonstrate various movement patterns
  demonstratePathSegments();
}

void loop() {
  // Update servo for smooth movements
  pen_servo.update();
  delay(10);
}

void demonstratePathSegments() {
  Serial.println("\n=== Path Segment Demonstrations ===");
  
  // 1. Straight movement - both motors same speed/direction
  Serial.println("1. Straight movement - synchronized motors");
  pen_servo.sweepTo(config.pen_up_angle, 500);
  delay(600);
  
  executeSegment(100, 100, 200, 200, false);  // left_steps, right_steps, left_speed, right_speed, pen_down
  delay(1000);
  
  // 2. Gentle right curve - left motor faster
  Serial.println("2. Gentle right curve - left motor faster");
  pen_servo.sweepTo(config.pen_down_angle, 500);
  delay(600);
  
  executeSegment(150, 100, 300, 200, true);  // Left faster for right curve
  delay(1000);
  
  // 3. Sharp left turn - right motor faster
  Serial.println("3. Sharp left turn - right motor faster");
  executeSegment(80, 150, 200, 400, true);  // Right faster for left turn
  delay(1000);
  
  // 4. Spin in place - motors opposite directions
  Serial.println("4. Spin in place - opposite motor directions");
  pen_servo.sweepTo(config.pen_up_angle, 400);
  delay(500);
  
  executeSpinSegment(100, 250, true);  // steps, speed, clockwise
  delay(1000);
  
  // 5. S-curve using multiple segments
  Serial.println("5. S-curve pattern using multiple segments");
  pen_servo.sweepTo(config.pen_down_angle, 500);
  delay(600);
  
  // First curve - right
  executeSegment(120, 80, 300, 200, true);
  delay(200);
  
  // Transition segment - straight
  executeSegment(50, 50, 250, 250, true);
  delay(200);
  
  // Second curve - left
  executeSegment(80, 120, 200, 300, true);
  delay(1000);
  
  // 6. Figure-8 pattern (simplified)
  Serial.println("6. Figure-8 pattern");
  
  // First loop
  for (int i = 0; i < 4; i++) {
    executeSegment(60, 40, 300, 200, true);  // Quarter circle right
    delay(100);
  }
  
  delay(500);
  
  // Second loop (opposite direction)
  for (int i = 0; i < 4; i++) {
    executeSegment(40, 60, 200, 300, true);  // Quarter circle left
    delay(100);
  }
  
  delay(1000);
  
  // 7. Complex asymmetric pattern
  Serial.println("7. Complex asymmetric movement pattern");
  
  // Varying speed and direction changes
  executeSegment(200, 150, 400, 300, true);  // Fast curve
  delay(200);
  executeSegment(100, 180, 200, 350, true);  // Slower opposing curve
  delay(200);
  executeSegment(80, 80, 500, 500, true);    // Fast straight
  delay(200);
  executeSpinSegment(50, 300, false);        // Quick spin counter-clockwise
  delay(1000);
  
  // Return pen to up position
  pen_servo.sweepTo(config.pen_up_angle, 800);
  delay(1000);
  
  Serial.println("\n=== Path segment demonstrations complete! ===");
  
  // Power down motors
  left_motor.release();
  right_motor.release();
  Serial.println("Motors powered down");
}

// === MOVEMENT EXECUTION FUNCTIONS ===

void executeSegment(int left_steps, int right_steps, int left_speed, int right_speed, bool pen_down) {
  Serial.print("  Executing segment: L=");
  Serial.print(left_steps);
  Serial.print(" steps @ ");
  Serial.print(left_speed);
  Serial.print(" sps, R=");
  Serial.print(right_steps);
  Serial.print(" steps @ ");
  Serial.print(right_speed);
  Serial.print(" sps, Pen=");
  Serial.println(pen_down ? "DOWN" : "UP");
  
  // Set motor speeds
  left_motor.setSpeed(left_speed);
  right_motor.setSpeed(right_speed);
  
  // Execute steps
  int left_remaining = left_steps;
  int right_remaining = right_steps;
  
  while (left_remaining > 0 || right_remaining > 0) {
    // Step left motor if needed
    if (left_remaining > 0 && left_motor.stepForward()) {
      left_remaining--;
    }
    
    // Step right motor if needed
    if (right_remaining > 0 && right_motor.stepForward()) {
      right_remaining--;
    }
    
    // Update servo
    pen_servo.update();
    
    // Progress indicator
    static int progress_counter = 0;
    if (++progress_counter % 1000 == 0) {
      Serial.print(".");
    }
    
    delayMicroseconds(100);  // Small delay for stability
  }
  
  Serial.println(" done");
  
  // Update estimated position (simplified)
  updatePositionEstimate(left_steps, right_steps);
}

void executeSpinSegment(int steps, int speed, bool clockwise) {
  Serial.print("  Spinning ");
  Serial.print(clockwise ? "clockwise" : "counter-clockwise");
  Serial.print(" for ");
  Serial.print(steps);
  Serial.println(" steps");
  
  // Set same speed for both motors
  left_motor.setSpeed(speed);
  right_motor.setSpeed(speed);
  
  for (int i = 0; i < steps; i++) {
    // Wait for both motors to be ready
    while (!left_motor.isReady() || !right_motor.isReady()) {
      pen_servo.update();
      delayMicroseconds(10);
    }
    
    if (clockwise) {
      // Clockwise: left forward, right backward
      left_motor.stepForward();
      right_motor.stepBackward();
    } else {
      // Counter-clockwise: left backward, right forward
      left_motor.stepBackward();
      right_motor.stepForward();
    }
    
    // Progress indicator
    if (i % 20 == 0) {
      Serial.print(".");
    }
  }
  
  Serial.println(" done");
  
  // Update angle estimate
  float angle_change = (steps * config.getMMPerStep()) / (config.wheelbase_mm / 2.0);
  if (!clockwise) angle_change = -angle_change;
  robot_position = robot_position.rotate(angle_change);
}

void updatePositionEstimate(int left_steps, int right_steps) {
  // Simple position estimation based on average wheel movement
  float mm_per_step = config.getMMPerStep();
  float avg_distance = ((left_steps + right_steps) / 2.0) * mm_per_step;
  
  // Move forward in current direction
  robot_position = robot_position.moveForward(avg_distance);
  
  // Estimate angle change from differential
  if (left_steps != right_steps) {
    float diff_distance = (right_steps - left_steps) * mm_per_step;
    float angle_change = diff_distance / config.wheelbase_mm;
    robot_position = robot_position.rotate(angle_change);
  }
  
  // Print estimated position occasionally
  static int update_count = 0;
  if (++update_count % 3 == 0) {
    Serial.print("    Estimated position: ");
    robot_position.print();
  }
}