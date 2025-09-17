#include "RobotMotionControl.h"

// MotorController Implementation
MotorController::MotorController() : currentStep(0), lastStepTime(0), stepDelay(1000) {
  for (int i = 0; i < 4; i++) {
    pins[i] = -1;
  }
}

void MotorController::begin(int pin1, int pin2, int pin3, int pin4) {
  pins[0] = pin1;
  pins[1] = pin2;
  pins[2] = pin3;
  pins[3] = pin4;
  
  for (int i = 0; i < 4; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }
}

void MotorController::setSpeed(int stepsPerSecond) {
  if (stepsPerSecond > 0) {
    stepDelay = 1000000L / stepsPerSecond; // microseconds per step
  }
}

void MotorController::step(int steps) {
  for (int i = 0; i < abs(steps); i++) {
    stepAsync(steps > 0 ? 1 : -1);
    delayMicroseconds(stepDelay);
  }
}

bool MotorController::stepAsync(int direction) {
  unsigned long now = micros();
  if (now - lastStepTime >= stepDelay) {
    // Update step position
    currentStep += direction;
    if (currentStep >= 8) currentStep = 0;
    if (currentStep < 0) currentStep = 7;
    
    // Apply step pattern
    for (int i = 0; i < 4; i++) {
      digitalWrite(pins[i], stepSequence[currentStep][i]);
    }
    
    lastStepTime = now;
    return true; // Step executed
  }
  return false; // Step not ready yet
}

void MotorController::stop() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(pins[i], LOW);
  }
}

// RobotMotionControl Implementation
RobotMotionControl::RobotMotionControl() : 
  currentX(0), currentY(0), currentAngle(0), penIsDown(false),
  executingSegment(false), leftStepsRemaining(0), rightStepsRemaining(0) {
}

RobotMotionControl::RobotMotionControl(const RobotConfig& cfg) : 
  config(cfg), currentX(0), currentY(0), currentAngle(0), penIsDown(false),
  executingSegment(false), leftStepsRemaining(0), rightStepsRemaining(0) {
}

void RobotMotionControl::begin() {
  // Initialize motors
  leftMotor.begin(config.motorLeftPins[0], config.motorLeftPins[1], 
                  config.motorLeftPins[2], config.motorLeftPins[3]);
  rightMotor.begin(config.motorRightPins[0], config.motorRightPins[1],
                   config.motorRightPins[2], config.motorRightPins[3]);
  
  // Initialize servo
  penServo.attach(config.servoPin);
  penUp(); // Start with pen up
  
  // Set default speeds
  leftMotor.setSpeed(500);
  rightMotor.setSpeed(500);
}

void RobotMotionControl::configure(const RobotConfig& cfg) {
  config = cfg;
}

void RobotMotionControl::executeSegment(const PathSegment& segment) {
  // Set servo position
  penServo.write(segment.servoAngle);
  delay(500); // Allow servo to move
  
  // Set motor speeds
  leftMotor.setSpeed(segment.motorLeftSpeed);
  rightMotor.setSpeed(segment.motorRightSpeed);
  
  // Calculate steps for each motor
  int leftSteps = segment.motorLeftMagnitude * segment.motorLeftDirection;
  int rightSteps = segment.motorRightMagnitude * segment.motorRightDirection;
  
  // Execute steps synchronously
  int maxSteps = max(abs(leftSteps), abs(rightSteps));
  
  for (int i = 0; i < maxSteps; i++) {
    bool stepLeft = (i < abs(leftSteps));
    bool stepRight = (i < abs(rightSteps));
    
    if (stepLeft) {
      leftMotor.stepAsync(leftSteps > 0 ? 1 : -1);
    }
    if (stepRight) {
      rightMotor.stepAsync(rightSteps > 0 ? 1 : -1);
    }
    
    // Wait for slowest motor
    delay(max(1000 / segment.motorLeftSpeed, 1000 / segment.motorRightSpeed));
  }
  
  // Update position estimate (simplified)
  updatePositionEstimate(leftSteps, rightSteps);
}

bool RobotMotionControl::executeSegmentAsync(const PathSegment& segment) {
  if (!executingSegment) {
    // Start new segment
    currentSegment = segment;
    leftStepsRemaining = segment.motorLeftMagnitude;
    rightStepsRemaining = segment.motorRightMagnitude;
    executingSegment = true;
    
    // Set servo position
    penServo.write(segment.servoAngle);
    
    // Set motor speeds
    leftMotor.setSpeed(segment.motorLeftSpeed);
    rightMotor.setSpeed(segment.motorRightSpeed);
    
    return false; // Not finished yet
  }
  
  return leftStepsRemaining == 0 && rightStepsRemaining == 0;
}

void RobotMotionControl::update() {
  if (!executingSegment) return;
  
  // Step motors if needed
  if (leftStepsRemaining > 0) {
    if (leftMotor.stepAsync(currentSegment.motorLeftDirection)) {
      leftStepsRemaining--;
    }
  }
  
  if (rightStepsRemaining > 0) {
    if (rightMotor.stepAsync(currentSegment.motorRightDirection)) {
      rightStepsRemaining--;
    }
  }
  
  // Check if segment is complete
  if (leftStepsRemaining == 0 && rightStepsRemaining == 0) {
    executingSegment = false;
    // Update position estimate
    updatePositionEstimate(
      currentSegment.motorLeftMagnitude * currentSegment.motorLeftDirection,
      currentSegment.motorRightMagnitude * currentSegment.motorRightDirection
    );
  }
}

void RobotMotionControl::moveStraight(float distanceMm, int speed, bool penDown) {
  PathSegment segment = createStraightSegment(distanceMm, speed, penDown);
  executeSegment(segment);
}

void RobotMotionControl::rotate(float angleDegrees, int speed) {
  PathSegment segment = createRotationSegment(angleDegrees, speed);
  executeSegment(segment);
}

void RobotMotionControl::penUp() {
  penServo.write(config.servoPenUpAngle);
  penIsDown = false;
  delay(500);
}

void RobotMotionControl::penDown() {
  penServo.write(config.servoPenDownAngle);
  penIsDown = true;
  delay(500);
}

void RobotMotionControl::setPenAngle(int angle) {
  penServo.write(angle);
  penIsDown = (angle == config.servoPenDownAngle);
  delay(500);
}

void RobotMotionControl::stop() {
  leftMotor.stop();
  rightMotor.stop();
  executingSegment = false;
}

void RobotMotionControl::home() {
  // Simple implementation - move to origin
  currentX = 0;
  currentY = 0;
  currentAngle = 0;
}

void RobotMotionControl::stepsToDistance(int steps, float& distance) {
  float wheelCircumference = PI * config.wheelDiameterMm;
  distance = (steps * wheelCircumference) / config.stepsPerRevolution;
}

void RobotMotionControl::distanceToSteps(float distance, int& steps) {
  float wheelCircumference = PI * config.wheelDiameterMm;
  steps = (distance * config.stepsPerRevolution) / wheelCircumference;
}

PathSegment RobotMotionControl::createStraightSegment(float distanceMm, int speed, bool penDown) {
  int steps;
  distanceToSteps(abs(distanceMm), steps);
  
  int direction = (distanceMm >= 0) ? DIRECTION_FORWARD : DIRECTION_BACKWARD;
  int servoAngle = penDown ? config.servoPenDownAngle : config.servoPenUpAngle;
  
  return PathSegment(direction, steps, speed, direction, steps, speed, servoAngle);
}

PathSegment RobotMotionControl::createRotationSegment(float angleDegrees, int speed) {
  // Calculate arc length for rotation
  float arcLength = (abs(angleDegrees) * PI / 180.0) * (config.wheelbaseMm / 2.0);
  
  int steps;
  distanceToSteps(arcLength, steps);
  
  int leftDirection, rightDirection;
  if (angleDegrees > 0) { // Clockwise rotation
    leftDirection = DIRECTION_FORWARD;
    rightDirection = DIRECTION_BACKWARD;
  } else { // Counter-clockwise rotation
    leftDirection = DIRECTION_BACKWARD;
    rightDirection = DIRECTION_FORWARD;
  }
  
  return PathSegment(leftDirection, steps, speed, rightDirection, steps, speed, 
                    penIsDown ? config.servoPenDownAngle : config.servoPenUpAngle);
}

void RobotMotionControl::updatePositionEstimate(int leftSteps, int rightSteps) {
  // Simplified differential drive kinematics
  float leftDistance, rightDistance;
  stepsToDistance(leftSteps, leftDistance);
  stepsToDistance(rightSteps, rightDistance);
  
  float distance = (leftDistance + rightDistance) / 2.0;
  float deltaAngle = (rightDistance - leftDistance) / config.wheelbaseMm;
  
  // Update position
  currentX += distance * cos(currentAngle);
  currentY += distance * sin(currentAngle);
  currentAngle += deltaAngle;
  
  // Normalize angle
  while (currentAngle > PI) currentAngle -= 2 * PI;
  while (currentAngle < -PI) currentAngle += 2 * PI;
}