/*
 * Combined Hardware Test Example
 * 
 * Tests both StepperDriver and ServoDriver working together
 * This simulates basic robot motion with pen control
 * 
 * Hardware Setup:
 * - Left stepper motor: pins 2, 3, 4, 5
 * - Right stepper motor: pins 6, 7, 8, 10
 * - Servo motor: pin 9
 * - Common 5V power and ground connections
 * 
 * Expected Behavior:
 * - Pen moves up, both motors step forward (robot moves forward)
 * - Pen moves down, both motors step backward (robot moves backward)
 * - Pen moves up, motors step in opposite directions (robot turns)
 * - Demonstrates coordinated hardware operation
 */

#include <TerraPenMotionControl.h>

// Hardware drivers
StepperDriver left_motor;
StepperDriver right_motor;
ServoDriver pen_servo;

// Use configuration for consistent setup
// Access global config with g_config.hardware

// Test state
enum TestPhase {
  INIT,
  PEN_UP_FORWARD,
  PEN_DOWN_BACKWARD, 
  PEN_UP_TURN_LEFT,
  PEN_DOWN_TURN_RIGHT,
  REST_PHASE
};

TestPhase current_phase = INIT;
unsigned long phase_start_time = 0;
int step_count = 0;
const int STEPS_PER_PHASE = 50;

void setup() {
  Serial.begin(115200);
  Serial.println("=== Combined Hardware Test ===");
  
  // Print configuration being used
  config.printConfig();
  
  // Initialize hardware using config values
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
  left_motor.setSpeed(100);   // 100 steps/second
  right_motor.setSpeed(100);
  
  Serial.println("All hardware initialized");
  Serial.println("Starting coordinated test...");
  Serial.println();
  
  current_phase = INIT;
  phase_start_time = millis();
  step_count = 0;
}

void loop() {
  // Always update servo for smooth movement
  pen_servo.update();
  
  // State machine for coordinated test
  switch (current_phase) {
    case INIT:
      if (millis() - phase_start_time > 2000) {  // 2 second initialization
        Serial.println("Phase 1: Pen UP + Forward Movement");
        pen_servo.sweepTo(config.pen_up_angle, 1000);  // Pen up over 1 second
        current_phase = PEN_UP_FORWARD;
        phase_start_time = millis();
        step_count = 0;
      }
      break;
      
    case PEN_UP_FORWARD:
      // Move both motors forward (robot goes straight)
      bool left_stepped = left_motor.stepForward();
      bool right_stepped = right_motor.stepForward();
      
      if (left_stepped || right_stepped) {
        step_count++;
        
        if (step_count % 10 == 0) {
          Serial.print("Forward steps: ");
          Serial.print(step_count);
          Serial.print(" / ");
          Serial.print(STEPS_PER_PHASE);
          Serial.print(" - Pen angle: ");
          Serial.println(pen_servo.getCurrentAngle());
        }
      }
      
      if (step_count >= STEPS_PER_PHASE) {
        Serial.println("Phase 2: Pen DOWN + Backward Movement");
        pen_servo.sweepTo(config.pen_down_angle, 1000);  // Pen down over 1 second
        current_phase = PEN_DOWN_BACKWARD;
        phase_start_time = millis();
        step_count = 0;
      }
      break;
      
    case PEN_DOWN_BACKWARD:
      // Move both motors backward (robot goes backward)
      left_stepped = left_motor.stepBackward();
      right_stepped = right_motor.stepBackward();
      
      if (left_stepped || right_stepped) {
        step_count++;
        
        if (step_count % 10 == 0) {
          Serial.print("Backward steps: ");
          Serial.print(step_count);
          Serial.print(" / ");
          Serial.print(STEPS_PER_PHASE);
          Serial.print(" - Pen angle: ");
          Serial.println(pen_servo.getCurrentAngle());
        }
      }
      
      if (step_count >= STEPS_PER_PHASE) {
        Serial.println("Phase 3: Pen UP + Turn Left");
        pen_servo.sweepTo(config.pen_up_angle, 800);  // Pen up quickly
        current_phase = PEN_UP_TURN_LEFT;
        phase_start_time = millis();
        step_count = 0;
      }
      break;
      
    case PEN_UP_TURN_LEFT:
      // Turn left: left motor backward, right motor forward
      left_stepped = left_motor.stepBackward();
      right_stepped = right_motor.stepForward();
      
      if (left_stepped || right_stepped) {
        step_count++;
        
        if (step_count % 10 == 0) {
          Serial.print("Turn left steps: ");
          Serial.print(step_count);
          Serial.print(" / ");
          Serial.print(STEPS_PER_PHASE);
          Serial.print(" - Pen angle: ");
          Serial.println(pen_servo.getCurrentAngle());
        }
      }
      
      if (step_count >= STEPS_PER_PHASE) {
        Serial.println("Phase 4: Pen DOWN + Turn Right");
        pen_servo.sweepTo(config.pen_down_angle, 800);  // Pen down quickly
        current_phase = PEN_DOWN_TURN_RIGHT;
        phase_start_time = millis();
        step_count = 0;
      }
      break;
      
    case PEN_DOWN_TURN_RIGHT:
      // Turn right: left motor forward, right motor backward
      left_stepped = left_motor.stepForward();
      right_stepped = right_motor.stepBackward();
      
      if (left_stepped || right_stepped) {
        step_count++;
        
        if (step_count % 10 == 0) {
          Serial.print("Turn right steps: ");
          Serial.print(step_count);
          Serial.print(" / ");
          Serial.print(STEPS_PER_PHASE);
          Serial.print(" - Pen angle: ");
          Serial.println(pen_servo.getCurrentAngle());
        }
      }
      
      if (step_count >= STEPS_PER_PHASE) {
        Serial.println("Phase 5: Rest - All motors stop");
        pen_servo.sweepTo(config.pen_up_angle, 1500);  // Pen up slowly
        left_motor.release();   // Turn off motor coils
        right_motor.release();
        current_phase = REST_PHASE;
        phase_start_time = millis();
      }
      break;
      
    case REST_PHASE:
      if (millis() - phase_start_time > 5000) {  // 5 second rest
        Serial.println();
        Serial.println("=== Test cycle complete! ===");
        Serial.println("Restarting in 3 seconds...");
        Serial.println();
        
        // Re-enable motors and restart
        left_motor.hold();
        right_motor.hold();
        current_phase = INIT;
        phase_start_time = millis() + 3000;  // 3 second delay
      }
      break;
  }
  
  // Small delay for stability
  delay(1);
}

/*
 * Expected Behavior:
 * 1. 2-second initialization with all systems ready
 * 2. Pen moves up, both motors step forward (simulates moving while pen up)
 * 3. Pen moves down, both motors step backward (simulates drawing while moving backward)
 * 4. Pen moves up, motors turn left (left back, right forward)
 * 5. Pen moves down, motors turn right (left forward, right back)
 * 6. 5-second rest with motors released to save power
 * 7. Cycle repeats
 * 
 * This test validates:
 * - Coordinated operation of all hardware
 * - Non-blocking driver behavior
 * - Configuration system usage
 * - Basic differential drive movements
 * - Pen control integration
 * 
 * Troubleshooting:
 * - Only one motor moves: Check wiring of non-moving motor
 * - Servo doesn't move: Check servo power and signal wire
 * - Irregular timing: Check Serial output for timing issues
 * - System resets: Check power supply capacity
 */