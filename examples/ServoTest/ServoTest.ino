/*
 * ServoDriver Test Example
 * 
 * Tests the ServoDriver class with smooth movement and state tracking
 * 
 * Hardware Setup:
 * - Connect servo motor signal wire to Arduino pin 9
 * - Connect servo power (red) to 5V
 * - Connect servo ground (black/brown) to GND
 * 
 * Expected Behavior:
 * - Servo moves smoothly between different positions
 * - Serial output shows movement progress and status
 * - Tests both immediate and smooth movement modes
 */

#include <TerraPenMotionControl.h>

// Create servo driver instance
ServoDriver servo;

// Test state machine
enum TestState {
  INIT,
  MOVE_TO_90,
  SWEEP_TO_0,
  SWEEP_TO_180,
  SWEEP_TO_CENTER,
  IMMEDIATE_MOVES,
  TEST_COMPLETE
};

TestState current_state = INIT;
unsigned long state_start_time = 0;
int immediate_move_count = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("=== ServoDriver Test ===");
  
  // Initialize servo on pin 9, starting at 90 degrees
  servo.begin(9, 90);
  
  Serial.println("Servo initialized at 90 degrees");
  Serial.println("Starting test sequence...");
  Serial.println();
  
  current_state = INIT;
  state_start_time = millis();
}

void loop() {
  // Always update servo (handles smooth movements)
  servo.update();
  
  // State machine for test sequence
  switch (current_state) {
    case INIT:
      if (millis() - state_start_time > 2000) {  // Wait 2 seconds
        Serial.println("Test 1: Immediate move to 90 degrees");
        servo.setAngle(90);
        current_state = MOVE_TO_90;
        state_start_time = millis();
      }
      break;
      
    case MOVE_TO_90:
      if (millis() - state_start_time > 1000) {  // Wait 1 second
        Serial.println("Test 2: Smooth sweep to 0 degrees over 2 seconds");
        servo.sweepTo(0, 2000);
        current_state = SWEEP_TO_0;
        state_start_time = millis();
      }
      break;
      
    case SWEEP_TO_0:
      // Print progress while moving
      if (servo.isMoving() && (millis() - state_start_time) % 200 == 0) {
        Serial.print("Progress: ");
        Serial.print(servo.getProgress() * 100, 1);
        Serial.print("% - Current angle: ");
        Serial.println(servo.getCurrentAngle());
      }
      
      if (!servo.isMoving()) {
        Serial.println("Reached 0 degrees");
        Serial.println("Test 3: Smooth sweep to 180 degrees over 3 seconds");
        servo.sweepTo(180, 3000);
        current_state = SWEEP_TO_180;
        state_start_time = millis();
      }
      break;
      
    case SWEEP_TO_180:
      // Print progress while moving
      if (servo.isMoving() && (millis() - state_start_time) % 300 == 0) {
        Serial.print("Progress: ");
        Serial.print(servo.getProgress() * 100, 1);
        Serial.print("% - Current angle: ");
        Serial.println(servo.getCurrentAngle());
      }
      
      if (!servo.isMoving()) {
        Serial.println("Reached 180 degrees");
        Serial.println("Test 4: Smooth sweep back to center over 1.5 seconds");
        servo.sweepTo(90, 1500);
        current_state = SWEEP_TO_CENTER;
        state_start_time = millis();
      }
      break;
      
    case SWEEP_TO_CENTER:
      if (!servo.isMoving()) {
        Serial.println("Back to center (90 degrees)");
        Serial.println("Test 5: Rapid immediate movements");
        current_state = IMMEDIATE_MOVES;
        state_start_time = millis();
        immediate_move_count = 0;
      }
      break;
      
    case IMMEDIATE_MOVES:
      if (millis() - state_start_time > 500) {  // Every 500ms
        immediate_move_count++;
        
        if (immediate_move_count <= 5) {
          int target_angle = (immediate_move_count % 2 == 0) ? 45 : 135;
          Serial.print("Immediate move ");
          Serial.print(immediate_move_count);
          Serial.print(" to ");
          Serial.print(target_angle);
          Serial.println(" degrees");
          servo.setAngle(target_angle);
          state_start_time = millis();
        } else {
          Serial.println("Immediate moves complete");
          servo.setAngle(90);  // Return to center
          current_state = TEST_COMPLETE;
          state_start_time = millis();
        }
      }
      break;
      
    case TEST_COMPLETE:
      if (millis() - state_start_time > 2000) {  // Wait 2 seconds
        Serial.println();
        Serial.println("=== All tests complete! ===");
        Serial.println("Test sequence will restart in 5 seconds...");
        Serial.println();
        
        // Restart test sequence
        current_state = INIT;
        state_start_time = millis() + 3000;  // 5 second delay
      }
      break;
  }
  
  // Print status every 5 seconds during long pauses
  static unsigned long last_status = 0;
  if (millis() - last_status > 5000) {
    Serial.print("Status - Angle: ");
    Serial.print(servo.getCurrentAngle());
    Serial.print("°, Target: ");
    Serial.print(servo.getTargetAngle());
    Serial.print("°, Moving: ");
    Serial.println(servo.isMoving() ? "Yes" : "No");
    last_status = millis();
  }
}

/*
 * Expected Behavior:
 * 1. Servo starts at 90° (center position)
 * 2. Immediate move to 90° (should be instant)
 * 3. Smooth 2-second sweep from 90° to 0°
 * 4. Smooth 3-second sweep from 0° to 180°
 * 5. Smooth 1.5-second sweep back to 90°
 * 6. Series of rapid immediate moves between 45° and 135°
 * 7. Return to center and restart sequence
 * 
 * Troubleshooting:
 * - No movement: Check servo wiring and power
 * - Jittery movement: Check power supply, add capacitor
 * - Wrong direction: Normal - servo direction varies by model
 * - Mechanical binding: Check servo isn't overloaded
 */