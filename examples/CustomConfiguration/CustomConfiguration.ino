/*
 * Custom Configuration Example
 * 
 * Demonstrates how to create and use custom robot configurations for 
 * different hardware setups, wheel sizes, pin assignments, and servo positions.
 * 
 * This example shows how to adapt the library to your specific robot
 * hardware configuration and test the resulting parameters.
 * 
 * Hardware Variations Demonstrated:
 * - Custom wheel diameter and wheelbase
 * - Different pin assignments  
 * - Custom servo angles for pen positions
 * - Performance parameter tuning
 */

#include <TerraPenMotionControl.h>

// Hardware drivers
StepperDriver left_motor;
StepperDriver right_motor;
ServoDriver pen_servo;

// Create custom configuration
RobotConfig customConfig;

void setup() {
  Serial.begin(115200);
  Serial.println("=== Custom Configuration Example ===");
  Serial.println("Demonstrates custom robot hardware configuration");
  Serial.println();
  
  // Set up custom configuration parameters
  setupCustomConfiguration();
  
  // Initialize hardware with custom configuration
  initializeHardware();
  
  Serial.println("Robot initialized with custom configuration");
  delay(2000);
  
  // Test the custom configuration
  testConfiguration();
}

void loop() {
  // Update servo for smooth movements
  pen_servo.update();
  delay(10);
}

void setupCustomConfiguration() {
  Serial.println("Setting up custom configuration...");
  
  // === PHYSICAL ROBOT DIMENSIONS ===
  // Customize these for your specific robot build
  customConfig.wheel_diameter_mm = 32.0;     // 32mm diameter wheels (larger than default)
  customConfig.wheelbase_mm = 45.0;          // 45mm wheelbase (wider than default)
  customConfig.steps_per_revolution = 2048;  // Standard for 28BYJ-48
  
  // === CUSTOM PIN ASSIGNMENTS ===
  // Rearrange pins to avoid conflicts or match your wiring
  
  // Left motor - using pins 3,4,5,6 instead of default 2,3,4,5
  customConfig.left_motor_pins[0] = 3;   // IN1
  customConfig.left_motor_pins[1] = 4;   // IN2
  customConfig.left_motor_pins[2] = 5;   // IN3
  customConfig.left_motor_pins[3] = 6;   // IN4
  
  // Right motor - using pins 7,8,11,12 instead of default 6,7,8,10
  customConfig.right_motor_pins[0] = 7;  // IN1
  customConfig.right_motor_pins[1] = 8;  // IN2
  customConfig.right_motor_pins[2] = 11; // IN3
  customConfig.right_motor_pins[3] = 12; // IN4
  
  // === SERVO CONFIGURATION ===
  customConfig.servo_pin = 10;            // Servo on pin 10 instead of 9
  customConfig.pen_up_angle = 120;        // Custom pen up position (higher than default)
  customConfig.pen_down_angle = 45;       // Custom pen down position (different from default)
  
  // === PERFORMANCE PARAMETERS ===
  customConfig.max_speed_mms = 60.0;             // Faster than default 50mm/s
  customConfig.max_angular_speed_rad_s = 1.2;   // Faster rotation
  customConfig.default_speed_mms = 20.0;        // Faster default movement
  customConfig.default_draw_speed_mms = 12.0;   // Faster default drawing
  
  // === WORKSPACE LIMITS ===
  // Define safe operating area for your robot
  customConfig.min_x = -150.0;
  customConfig.max_x = 150.0;
  customConfig.min_y = -150.0; 
  customConfig.max_y = 150.0;
  
  Serial.println("Custom configuration complete!");
  Serial.println();
  
  // Display the configuration
  customConfig.printConfig();
  
  // Validate configuration
  if (customConfig.isValid()) {
    Serial.println("✓ Configuration validation passed");
  } else {
    Serial.println("✗ Configuration validation failed - check parameters!");
  }
  Serial.println();
}

void initializeHardware() {
  Serial.println("Initializing hardware with custom configuration...");
  
  // Initialize motors with custom pin assignments
  left_motor.begin(
    customConfig.left_motor_pins[0], 
    customConfig.left_motor_pins[1],
    customConfig.left_motor_pins[2], 
    customConfig.left_motor_pins[3]
  );
  
  right_motor.begin(
    customConfig.right_motor_pins[0], 
    customConfig.right_motor_pins[1],
    customConfig.right_motor_pins[2], 
    customConfig.right_motor_pins[3]
  );
  
  // Initialize servo with custom pin and starting position
  pen_servo.begin(customConfig.servo_pin, customConfig.pen_up_angle);
  
  // Set initial motor speeds based on custom configuration
  float initial_speed_steps_per_sec = customConfig.default_speed_mms * customConfig.getStepsPerMM();
  left_motor.setSpeed(initial_speed_steps_per_sec);
  right_motor.setSpeed(initial_speed_steps_per_sec);
  
  Serial.println("Hardware initialization complete");
  Serial.print("Initial motor speed: ");
  Serial.print(initial_speed_steps_per_sec);
  Serial.println(" steps/sec");
  Serial.println();
}

void testConfiguration() {
  Serial.println("=== Testing Custom Configuration ===");
  
  // Test 1: Servo movement with custom angles
  testServoMovement();
  delay(1000);
  
  // Test 2: Motor movement with custom parameters
  testMotorMovement();
  delay(1000);
  
  // Test 3: Calculated values verification
  testCalculatedValues();
  delay(1000);
  
  // Test 4: Workspace limits validation
  testWorkspaceLimits();
  delay(1000);
  
  // Test 5: Performance parameters
  testPerformanceParameters();
  
  Serial.println("\n=== Configuration testing complete! ===");
  
  // Power down
  left_motor.release();
  right_motor.release();
  Serial.println("Motors powered down");
}

void testServoMovement() {
  Serial.println("Test 1: Custom servo angles");
  Serial.print("Moving to pen UP position (");
  Serial.print(customConfig.pen_up_angle);
  Serial.println("°)");
  
  pen_servo.sweepTo(customConfig.pen_up_angle, 1000);
  delay(1200);
  
  Serial.print("Moving to pen DOWN position (");
  Serial.print(customConfig.pen_down_angle);
  Serial.println("°)");
  
  pen_servo.sweepTo(customConfig.pen_down_angle, 1000);
  delay(1200);
  
  Serial.println("Returning to UP position");
  pen_servo.sweepTo(customConfig.pen_up_angle, 800);
  delay(1000);
  
  Serial.println("✓ Servo movement test complete");
}

void testMotorMovement() {
  Serial.println("Test 2: Motor movement with custom wheel parameters");
  
  // Calculate steps for 50mm movement using custom wheel diameter
  float distance_mm = 50.0;
  int steps_needed = distance_mm * customConfig.getStepsPerMM();
  
  Serial.print("Moving forward ");
  Serial.print(distance_mm);
  Serial.print("mm (");
  Serial.print(steps_needed);
  Serial.println(" steps with custom wheel diameter)");
  
  // Move both motors forward
  for (int i = 0; i < steps_needed; i++) {
    while (!left_motor.isReady() || !right_motor.isReady()) {
      pen_servo.update();
      delayMicroseconds(10);
    }
    
    left_motor.stepForward();
    right_motor.stepForward();
    
    if (i % 50 == 0) Serial.print(".");
  }
  Serial.println(" done");
  
  delay(500);
  
  // Test rotation with custom wheelbase
  float turn_angle_deg = 90.0;
  float turn_angle_rad = turn_angle_deg * PI / 180.0;
  float arc_length = turn_angle_rad * (customConfig.wheelbase_mm / 2.0);
  int turn_steps = arc_length * customConfig.getStepsPerMM();
  
  Serial.print("Turning ");
  Serial.print(turn_angle_deg);
  Serial.print("° (");
  Serial.print(turn_steps);
  Serial.println(" steps with custom wheelbase)");
  
  // Turn in place
  for (int i = 0; i < turn_steps; i++) {
    while (!left_motor.isReady() || !right_motor.isReady()) {
      pen_servo.update();
      delayMicroseconds(10);
    }
    
    left_motor.stepBackward();  // Left backward
    right_motor.stepForward();  // Right forward
    
    if (i % 20 == 0) Serial.print(".");
  }
  Serial.println(" done");
  
  Serial.println("✓ Motor movement test complete");
}

void testCalculatedValues() {
  Serial.println("Test 3: Calculated values verification");
  
  float wheel_circumference = PI * customConfig.wheel_diameter_mm;
  float mm_per_step = customConfig.getMMPerStep();
  float steps_per_mm = customConfig.getStepsPerMM();
  
  Serial.print("Wheel circumference: ");
  Serial.print(wheel_circumference, 2);
  Serial.println(" mm");
  
  Serial.print("Distance per step: ");
  Serial.print(mm_per_step, 4);
  Serial.println(" mm");
  
  Serial.print("Steps per mm: ");
  Serial.print(steps_per_mm, 2);
  Serial.println(" steps");
  
  // Test calculations
  float test_distance = 100.0;  // 100mm
  int calculated_steps = test_distance * steps_per_mm;
  float back_calculated_distance = calculated_steps * mm_per_step;
  
  Serial.print("Test: ");
  Serial.print(test_distance);
  Serial.print("mm → ");
  Serial.print(calculated_steps);
  Serial.print(" steps → ");
  Serial.print(back_calculated_distance, 2);
  Serial.println("mm");
  
  float error = abs(test_distance - back_calculated_distance);
  Serial.print("Calculation error: ");
  Serial.print(error, 4);
  Serial.println(" mm");
  
  if (error < 0.01) {
    Serial.println("✓ Calculation accuracy test passed");
  } else {
    Serial.println("✗ Calculation accuracy test failed");
  }
}

void testWorkspaceLimits() {
  Serial.println("Test 4: Workspace limits validation");
  
  // Test various coordinates
  Position test_positions[] = {
    Position(0, 0),           // Center (should be valid)
    Position(100, 100),       // Within limits (should be valid)  
    Position(200, 200),       // Outside limits (should be invalid)
    Position(-100, -100),     // Within limits (should be valid)
    Position(-200, -200)      // Outside limits (should be invalid)
  };
  
  for (int i = 0; i < 5; i++) {
    Position pos = test_positions[i];
    bool is_valid = customConfig.isInWorkspace(pos.x, pos.y);
    
    Serial.print("Position (");
    Serial.print(pos.x);
    Serial.print(", ");
    Serial.print(pos.y);
    Serial.print("): ");
    Serial.println(is_valid ? "✓ Valid" : "✗ Outside workspace");
  }
  
  Serial.println("✓ Workspace limits test complete");
}

void testPerformanceParameters() {
  Serial.println("Test 5: Performance parameters");
  
  // Test speed conversion
  float speed_mms = customConfig.default_speed_mms;
  float speed_steps_per_sec = speed_mms * customConfig.getStepsPerMM();
  
  Serial.print("Default speed: ");
  Serial.print(speed_mms);
  Serial.print(" mm/s = ");
  Serial.print(speed_steps_per_sec);
  Serial.println(" steps/sec");
  
  // Test maximum speeds
  Serial.print("Maximum linear speed: ");
  Serial.print(customConfig.max_speed_mms);
  Serial.println(" mm/s");
  
  Serial.print("Maximum angular speed: ");
  Serial.print(customConfig.max_angular_speed_rad_s);
  Serial.print(" rad/s = ");
  Serial.print(customConfig.max_angular_speed_rad_s * 180.0 / PI);
  Serial.println(" deg/s");
  
  // Test if current motor speed is within limits
  float current_motor_speed = left_motor.getSpeed();
  float max_motor_speed = customConfig.max_speed_mms * customConfig.getStepsPerMM();
  
  Serial.print("Current motor speed: ");
  Serial.print(current_motor_speed);
  Serial.println(" steps/sec");
  
  Serial.print("Maximum motor speed: ");
  Serial.print(max_motor_speed);
  Serial.println(" steps/sec");
  
  if (current_motor_speed <= max_motor_speed) {
    Serial.println("✓ Motor speed within limits");
  } else {
    Serial.println("✗ Motor speed exceeds limits");
  }
  
  Serial.println("✓ Performance parameters test complete");
}