#ifndef ROBOT_MOTION_CONTROL_H
#define ROBOT_MOTION_CONTROL_H

#include <Arduino.h>
#include <Servo.h>

// Motor direction constants
#define DIRECTION_FORWARD  1
#define DIRECTION_BACKWARD -1

// Servo position constants  
#define PEN_UP   90
#define PEN_DOWN 0

// Default configuration values
#define DEFAULT_WHEEL_DIAMETER_MM     25.0
#define DEFAULT_WHEELBASE_MM          30.0
#define DEFAULT_STEPS_PER_REVOLUTION  2048  // 28BYJ stepper with gearing
#define DEFAULT_MAX_STEP_FREQUENCY_HZ 1000
#define DEFAULT_SERVO_PIN             9

/**
 * Robot hardware configuration structure
 */
struct RobotConfig {
  // Physical dimensions
  float wheelDiameterMm;
  float wheelbaseMm;
  
  // Motor configuration
  int stepsPerRevolution;
  int maxStepFrequencyHz;
  
  // Pin assignments
  int motorLeftPins[4];   // ULN2803A input pins for left motor
  int motorRightPins[4];  // ULN2803A input pins for right motor
  int servoPin;
  
  // Servo positions
  int servoPenUpAngle;
  int servoPenDownAngle;
  
  // Constructor with defaults
  RobotConfig() :
    wheelDiameterMm(DEFAULT_WHEEL_DIAMETER_MM),
    wheelbaseMm(DEFAULT_WHEELBASE_MM),
    stepsPerRevolution(DEFAULT_STEPS_PER_REVOLUTION),
    maxStepFrequencyHz(DEFAULT_MAX_STEP_FREQUENCY_HZ),
    servoPin(DEFAULT_SERVO_PIN),
    servoPenUpAngle(PEN_UP),
    servoPenDownAngle(PEN_DOWN) {
    
    // Default pin assignments (change as needed)
    motorLeftPins[0] = 2;   // IN1
    motorLeftPins[1] = 3;   // IN2  
    motorLeftPins[2] = 4;   // IN3
    motorLeftPins[3] = 5;   // IN4
    
    motorRightPins[0] = 6;  // IN1
    motorRightPins[1] = 7;  // IN2
    motorRightPins[2] = 8;  // IN3
    motorRightPins[3] = 9;  // IN4
  }
};

/**
 * Path segment definition for robot motion
 */
struct PathSegment {
  // Left motor parameters
  int motorLeftDirection;    // DIRECTION_FORWARD or DIRECTION_BACKWARD
  int motorLeftMagnitude;    // Number of steps
  int motorLeftSpeed;        // Steps per second
  
  // Right motor parameters  
  int motorRightDirection;   // DIRECTION_FORWARD or DIRECTION_BACKWARD
  int motorRightMagnitude;   // Number of steps
  int motorRightSpeed;       // Steps per second
  
  // Servo state
  int servoAngle;           // Servo position (PEN_UP/PEN_DOWN or custom angle)
  
  // Constructor
  PathSegment(int leftDir = DIRECTION_FORWARD, int leftMag = 0, int leftSpd = 500,
              int rightDir = DIRECTION_FORWARD, int rightMag = 0, int rightSpd = 500,
              int servo = PEN_UP) :
    motorLeftDirection(leftDir), motorLeftMagnitude(leftMag), motorLeftSpeed(leftSpd),
    motorRightDirection(rightDir), motorRightMagnitude(rightMag), motorRightSpeed(rightSpd),
    servoAngle(servo) {}
};

/**
 * Low-level motor controller for 28BYJ steppers via ULN2803A
 */
class MotorController {
private:
  int pins[4];
  int currentStep;
  unsigned long lastStepTime;
  int stepDelay;
  
  // 28BYJ stepper sequence (half-step mode for better resolution)
  const int stepSequence[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
  };
  
public:
  MotorController();
  void begin(int pin1, int pin2, int pin3, int pin4);
  void setSpeed(int stepsPerSecond);
  void step(int steps);
  bool stepAsync(int direction); // Non-blocking step function
  void stop();
};

/**
 * Main robot motion control class
 */
class RobotMotionControl {
private:
  RobotConfig config;
  MotorController leftMotor;
  MotorController rightMotor;
  Servo penServo;
  
  // Current state
  float currentX, currentY, currentAngle;
  bool penIsDown;
  
  // Async execution state
  bool executingSegment;
  PathSegment currentSegment;
  int leftStepsRemaining, rightStepsRemaining;
  unsigned long lastUpdateTime;
  
public:
  RobotMotionControl();
  RobotMotionControl(const RobotConfig& cfg);
  
  // Initialization
  void begin();
  void configure(const RobotConfig& cfg);
  
  // Basic movement commands
  void executeSegment(const PathSegment& segment);
  bool executeSegmentAsync(const PathSegment& segment); // Non-blocking
  void update(); // Call in loop() for async operations
  
  // High-level movement functions
  void moveStraight(float distanceMm, int speed = 500, bool penDown = false);
  void rotate(float angleDegrees, int speed = 500);
  void moveTo(float x, float y, int speed = 500, bool penDown = false);
  
  // Pen control
  void penUp();
  void penDown();
  void setPenAngle(int angle);
  
  // State queries
  float getX() const { return currentX; }
  float getY() const { return currentY; }
  float getAngle() const { return currentAngle; }
  bool isPenDown() const { return penIsDown; }
  bool isBusy() const { return executingSegment; }
  
  // Utility functions
  void stop();
  void home(); // Return to origin
  
  // Kinematics helpers
  void stepsToDistance(int steps, float& distance);
  void distanceToSteps(float distance, int& steps);
  PathSegment createStraightSegment(float distanceMm, int speed, bool penDown);
  PathSegment createRotationSegment(float angleDegrees, int speed);
};

#endif // ROBOT_MOTION_CONTROL_H