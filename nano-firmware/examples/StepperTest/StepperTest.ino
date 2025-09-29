/*
 * StepperDriver Test Example
 * 
 * Tests the StepperDriver class with a single 28BYJ-48 stepper motor
 * 
 * Hardware Setup:
 * - Connect 28BYJ-48 stepper motor to ULN2803A driver
 * - ULN2803A IN1-IN4 connected to Arduino pins 2-5
 * - Power: 5V to motor, GND common
 * 
 * Expected Behavior:
 * - Motor steps forward 100 steps, pauses
 * - Motor steps backward 100 steps, pauses
 * - Repeats cycle with speed changes
 * - Serial output shows step progress
 */

#include <TerraPenMotionControl.h>

// Create stepper driver instance
StepperDriver motor;

// Test parameters
int forward_steps = 0;
int backward_steps = 0;
bool going_forward = true;
const int STEPS_PER_CYCLE = 100;

void setup() {
  Serial.begin(115200);
  Serial.println("=== StepperDriver Test ===");
  
  // Initialize motor (pins 2, 3, 4, 5)
  motor.begin(2, 3, 4, 5);
  
  // Set initial speed (100 steps/second)
  motor.setSpeed(100);
  
  Serial.println("Motor initialized");
  Serial.print("Speed: ");
  Serial.print(motor.getSpeed());
  Serial.println(" steps/sec");
  Serial.println("Starting test cycle...");
  Serial.println();
}

void loop() {
  // Execute stepping based on current direction
  if (going_forward) {
    if (motor.stepForward()) {
      forward_steps++;
      
      // Print progress every 10 steps
      if (forward_steps % 10 == 0) {
        Serial.print("Forward: ");
        Serial.print(forward_steps);
        Serial.print(" / ");
        Serial.println(STEPS_PER_CYCLE);
      }
      
      // Check if forward cycle complete
      if (forward_steps >= STEPS_PER_CYCLE) {
        Serial.println("Forward cycle complete - switching to backward");
        going_forward = false;
        forward_steps = 0;
        delay(1000);  // Pause between cycles
      }
    }
  } else {
    if (motor.stepBackward()) {
      backward_steps++;
      
      // Print progress every 10 steps
      if (backward_steps % 10 == 0) {
        Serial.print("Backward: ");
        Serial.print(backward_steps);
        Serial.print(" / ");
        Serial.println(STEPS_PER_CYCLE);
      }
      
      // Check if backward cycle complete
      if (backward_steps >= STEPS_PER_CYCLE) {
        Serial.println("Backward cycle complete - switching to forward");
        
        // Change speed for next cycle
        float current_speed = motor.getSpeed();
        if (current_speed >= 200) {
          motor.setSpeed(50);   // Slow down
          Serial.println("Speed changed to 50 steps/sec");
        } else {
          motor.setSpeed(current_speed + 25);  // Speed up
          Serial.print("Speed changed to ");
          Serial.print(motor.getSpeed());
          Serial.println(" steps/sec");
        }
        
        going_forward = true;
        backward_steps = 0;
        delay(1000);  // Pause between cycles
      }
    }
  }
  
  // Small delay for stability
  delay(1);
}

/*
 * Expected Serial Output:
 * 
 * === StepperDriver Test ===
 * Motor initialized
 * Speed: 100.00 steps/sec
 * Starting test cycle...
 * 
 * Forward: 10 / 100
 * Forward: 20 / 100
 * ...
 * Forward: 100 / 100
 * Forward cycle complete - switching to backward
 * Backward: 10 / 100
 * ...
 * 
 * Troubleshooting:
 * - No movement: Check wiring, power supply
 * - Irregular movement: Check step timing, power supply stability
 * - Motor gets hot: Normal for stepper motors under load
 * - Wrong direction: Swap any two motor wire pairs
 */