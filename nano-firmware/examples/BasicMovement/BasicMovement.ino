/*
 * Basic Movement Example
 * 
 * Demonstrates basic robot movement patterns using individual hardware drivers.
 * This example shows coordinated movement of stepper motors and servo control
 * to create simple drawing robot behaviors.
 * 
 * Hardware Requirements:
 * - Arduino Nano
 * - Two 28BYJ-48 stepper motors
 * - ULN2803A driver IC
 * - Servo motor for pen control
 * 
 * Wiring (using default RobotConfig):
 * - Left motor: pins 2,3,4,5 -> ULN2803A inputs 1-4
 * - Right motor: pins 6,7,8,10 -> ULN2803A inputs 5-8
 * - Servo: pin 9
 * 
 * Movement Patterns Demonstrated:
 * - Forward movement (both motors forward)
 * - Backward movement (both motors backward)
 * - Left turn (left motor backward, right motor forward)
 * - Right turn (left motor forward, right motor backward)
 * - Pen control (up/down movements)
 */

#include <TerraPenMotionControl.h>

// Hardware drivers
StepperDriver left_motor;
StepperDriver right_motor;
ServoDriver pen_servo;

// Use configuration for consistent setup
RobotConfig config;

// Movement tracking
Position current_position(0, 0, 0);  // Start at origin facing forward

// Movement parameters
const int MOVEMENT_SPEED = 150;  // steps per second
const int STEPS_FOR_MOVEMENT = 100;  // steps per movement segment
const int STEPS_FOR_TURN = 50;       // steps per turn

void setup() {
  Serial.begin(115200);
  Serial.println("=== Basic Movement Example ===");
  Serial.println("Demonstrates coordinated robot movements");
  Serial.println();
  
  // Print configuration
  Serial.println("Using configuration:");
  config.printConfig();
  Serial.println();
  
  // Initialize hardware using config
  left_motor.begin(
    config.left_motor_pins[0], 
    config.left_motor_pins[1],
    config.left_motor_pins[2], 
    config.left_motor_pins[3]
  );
  
  right_motor.begin(
    config.right_motor_pins[0], 
    config.right_motor_pins[1],
    config.right_motor_pins[2], 
    config.right_motor_pins[3]
  );
  
  pen_servo.begin(config.servo_pin, config.pen_up_angle);
  
  // Set motor speeds
  left_motor.setSpeed(MOVEMENT_SPEED);
  right_motor.setSpeed(MOVEMENT_SPEED);
  
  Serial.println("Hardware initialized successfully");
  Serial.println("Starting movement demonstration in 3 seconds...");
  delay(3000);
  
  // Run movement demonstration
  demonstrateBasicMovements();
}

void loop() {
  // Update servo for smooth movements
  pen_servo.update();
  
  // This example runs once in setup, but you could add
  // continuous movement patterns here
  delay(100);
}

void demonstrateBasicMovements() {
  Serial.println("\n=== Movement Demonstration ===");
  
  // 1. Forward movement with pen up
  Serial.println("1. Moving forward (pen up)");
  pen_servo.sweepTo(config.pen_up_angle, 500);
  delay(600);  // Wait for pen to move
  
  moveForward(STEPS_FOR_MOVEMENT);
  updatePosition(STEPS_FOR_MOVEMENT, STEPS_FOR_MOVEMENT);
  delay(1000);
  
  // 2. Turn left 90 degrees
  Serial.println("2. Turning left 90 degrees");
  turnLeft(STEPS_FOR_TURN);
  current_position = current_position.rotate(PI/2);  // Update angle
  delay(1000);
  
  // 3. Forward movement with pen down (drawing)
  Serial.println("3. Moving forward while drawing (pen down)");
  pen_servo.sweepTo(config.pen_down_angle, 500);
  delay(600);  // Wait for pen to move
  
  moveForward(STEPS_FOR_MOVEMENT);
  updatePosition(STEPS_FOR_MOVEMENT, STEPS_FOR_MOVEMENT);
  delay(1000);
  
  // 4. Turn right 180 degrees
  Serial.println("4. Turning right 180 degrees (still drawing)");
  turnRight(STEPS_FOR_TURN * 2);  // Double steps for 180 degree turn
  current_position = current_position.rotate(-PI);  // Update angle
  delay(1000);
  
  // 5. Move backward to create a line
  Serial.println("5. Moving backward to complete line");
  moveBackward(STEPS_FOR_MOVEMENT);
  updatePosition(-STEPS_FOR_MOVEMENT, -STEPS_FOR_MOVEMENT);
  delay(1000);
  
  // 6. Pen up and return to start orientation
  Serial.println("6. Pen up and turn to original orientation");
  pen_servo.sweepTo(config.pen_up_angle, 500);
  delay(600);
  
  turnLeft(STEPS_FOR_TURN);  // Turn back to original direction
  current_position = current_position.rotate(PI/2);
  delay(1000);
  
  // 7. Return to approximate starting position
  Serial.println("7. Returning to starting area");
  moveBackward(STEPS_FOR_MOVEMENT);
  updatePosition(-STEPS_FOR_MOVEMENT, -STEPS_FOR_MOVEMENT);
  delay(1000);
  
  Serial.println("\n=== Movement demonstration complete! ===");
  Serial.print("Approximate final position: ");
  current_position.print();
  Serial.println("Note: Position tracking is estimated based on step counts");
  
  // Power down motors to save energy
  left_motor.release();
  right_motor.release();
  Serial.println("Motors released (powered down)");
}

// === MOVEMENT FUNCTIONS ===

void moveForward(int steps) {
  Serial.print("  Moving forward ");
  Serial.print(steps);
  Serial.println(" steps");
  
  for (int i = 0; i < steps; i++) {
    // Wait for both motors to be ready
    while (!left_motor.isReady() || !right_motor.isReady()) {
      pen_servo.update();  // Keep servo updating
      delayMicroseconds(10);
    }
    
    // Step both motors forward
    left_motor.stepForward();
    right_motor.stepForward();
    
    // Progress indicator
    if (i % 20 == 0) {
      Serial.print(".");
    }
  }
  Serial.println(" done");
}

void moveBackward(int steps) {
  Serial.print("  Moving backward ");
  Serial.print(steps);
  Serial.println(" steps");
  
  for (int i = 0; i < steps; i++) {
    // Wait for both motors to be ready
    while (!left_motor.isReady() || !right_motor.isReady()) {
      pen_servo.update();  // Keep servo updating
      delayMicroseconds(10);
    }
    
    // Step both motors backward
    left_motor.stepBackward();
    right_motor.stepBackward();
    
    // Progress indicator
    if (i % 20 == 0) {
      Serial.print(".");
    }
  }
  Serial.println(" done");
}

void turnLeft(int steps) {
  Serial.print("  Turning left ");
  Serial.print(steps);
  Serial.println(" steps");
  
  for (int i = 0; i < steps; i++) {
    // Wait for both motors to be ready
    while (!left_motor.isReady() || !right_motor.isReady()) {
      pen_servo.update();  // Keep servo updating
      delayMicroseconds(10);
    }
    
    // Left motor backward, right motor forward
    left_motor.stepBackward();
    right_motor.stepForward();
    
    // Progress indicator
    if (i % 10 == 0) {
      Serial.print(".");
    }
  }
  Serial.println(" done");
}

void turnRight(int steps) {
  Serial.print("  Turning right ");
  Serial.print(steps);
  Serial.println(" steps");
  
  for (int i = 0; i < steps; i++) {
    // Wait for both motors to be ready
    while (!left_motor.isReady() || !right_motor.isReady()) {
      pen_servo.update();  // Keep servo updating
      delayMicroseconds(10);
    }
    
    // Left motor forward, right motor backward
    left_motor.stepForward();
    right_motor.stepBackward();
    
    // Progress indicator
    if (i % 10 == 0) {
      Serial.print(".");
    }
  }
  Serial.println(" done");
}

void updatePosition(int left_steps, int right_steps) {
  // Simple position estimation based on differential drive kinematics
  // This is approximate - real implementation would be in robot control layer
  
  float mm_per_step = config.getMMPerStep();
  float left_distance = left_steps * mm_per_step;
  float right_distance = right_steps * mm_per_step;
  
  // Average distance for forward movement estimation
  float avg_distance = (left_distance + right_distance) / 2.0;
  
  // Move forward in current direction
  current_position = current_position.moveForward(avg_distance);
  
  Serial.print("  Estimated position: ");
  current_position.print();
}