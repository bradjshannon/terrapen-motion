/*
  Custom Configuration Example
  
  Demonstrates how to configure the robot library for different hardware
  setups, wheel sizes, pin assignments, and servo positions.
  
  This example shows how to adapt the library to your specific robot
  hardware configuration.
*/

#include <RobotMotionControl.h>

// Create custom configuration
RobotConfig myConfig;
RobotMotionControl robot;

void setup() {
  Serial.begin(9600);
  Serial.println("Robot Motion Control - Custom Configuration Example");
  
  // Configure hardware parameters
  setupCustomConfiguration();
  
  // Create robot with custom config
  robot.configure(myConfig);
  robot.begin();
  
  Serial.println("Robot initialized with custom configuration.");
  
  // Test the configuration
  testConfiguration();
}

void loop() {
  robot.update();
  delay(10);
}

void setupCustomConfiguration() {
  Serial.println("Setting up custom configuration...");
  
  // Physical robot dimensions
  myConfig.wheelDiameterMm = 30.0;     // 30mm diameter wheels
  myConfig.wheelbaseMm = 35.0;         // 35mm wheelbase
  
  // Motor specifications
  myConfig.stepsPerRevolution = 2048;   // 28BYJ with gearing
  myConfig.maxStepFrequencyHz = 800;    // Conservative max speed
  
  // Custom pin assignments for motors
  // Left motor pins (connect to ULN2803A inputs)
  myConfig.motorLeftPins[0] = 2;   // IN1
  myConfig.motorLeftPins[1] = 3;   // IN2
  myConfig.motorLeftPins[2] = 4;   // IN3
  myConfig.motorLeftPins[3] = 5;   // IN4
  
  // Right motor pins (connect to ULN2803A inputs)
  myConfig.motorRightPins[0] = 6;  // IN1
  myConfig.motorRightPins[1] = 7;  // IN2
  myConfig.motorRightPins[2] = 8;  // IN3
  myConfig.motorRightPins[3] = 9;  // IN4
  
  // Servo configuration
  myConfig.servoPin = 10;           // Servo control pin
  myConfig.servoPenUpAngle = 120;   // Custom pen up position
  myConfig.servoPenDownAngle = 60;  // Custom pen down position
  
  Serial.println("Configuration complete:");
  Serial.print("  Wheel diameter: ");
  Serial.print(myConfig.wheelDiameterMm);
  Serial.println(" mm");
  
  Serial.print("  Wheelbase: ");
  Serial.print(myConfig.wheelbaseMm);
  Serial.println(" mm");
  
  Serial.print("  Steps per revolution: ");
  Serial.println(myConfig.stepsPerRevolution);
  
  Serial.print("  Max step frequency: ");
  Serial.print(myConfig.maxStepFrequencyHz);
  Serial.println(" Hz");
  
  Serial.print("  Servo pin: ");
  Serial.println(myConfig.servoPin);
  
  Serial.print("  Pen up angle: ");
  Serial.println(myConfig.servoPenUpAngle);
  
  Serial.print("  Pen down angle: ");
  Serial.println(myConfig.servoPenDownAngle);
}

void testConfiguration() {
  Serial.println("\nTesting custom configuration...");
  
  // Test pen movement with custom angles
  Serial.println("Testing pen movement:");
  robot.penUp();
  delay(1000);
  robot.penDown();
  delay(1000);
  robot.penUp();
  delay(1000);
  
  // Test movement with custom wheel parameters
  Serial.println("Testing movement with custom parameters:");
  
  // Move forward - distance calculation uses custom wheel diameter
  Serial.println("Moving forward 50mm");
  robot.moveStraight(50, 400, false);
  delay(1000);
  
  // Rotate - calculation uses custom wheelbase
  Serial.println("Rotating 90 degrees");
  robot.rotate(90, 300);
  delay(1000);
  
  // Draw a small square using pen down
  Serial.println("Drawing a square:");
  robot.penDown();
  
  for (int i = 0; i < 4; i++) {
    robot.moveStraight(30, 300, true);  // 30mm sides
    delay(500);
    robot.rotate(90, 200);               // 90 degree turns
    delay(500);
  }
  
  robot.penUp();
  
  Serial.println("Configuration test complete!");
  
  // Display calculated values
  displayCalculatedValues();
}

void displayCalculatedValues() {
  Serial.println("\n--- Calculated Values ---");
  
  // Calculate wheel circumference
  float wheelCircumference = PI * myConfig.wheelDiameterMm;
  Serial.print("Wheel circumference: ");
  Serial.print(wheelCircumference);
  Serial.println(" mm");
  
  // Calculate mm per step
  float mmPerStep = wheelCircumference / myConfig.stepsPerRevolution;
  Serial.print("Distance per step: ");
  Serial.print(mmPerStep);
  Serial.println(" mm");
  
  // Calculate steps for 10mm
  int stepsFor10mm = (10.0 * myConfig.stepsPerRevolution) / wheelCircumference;
  Serial.print("Steps for 10mm: ");
  Serial.println(stepsFor10mm);
  
  // Calculate steps for 90 degree rotation
  float arcLength = (90.0 * PI / 180.0) * (myConfig.wheelbaseMm / 2.0);
  int stepsFor90deg = (arcLength * myConfig.stepsPerRevolution) / wheelCircumference;
  Serial.print("Steps for 90° rotation: ");
  Serial.println(stepsFor90deg);
  
  Serial.println("------------------------\n");
}