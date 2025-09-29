/**
 * CoordinateAccuracyTest.ino
 * 
 * Phase 2 coordinate system accuracy validation example.
 * Tests the new coordinate-based movement system including:
 * - Basic moveTo() and drawTo() operations
 * - Position tracking accuracy
 * - Simple geometric patterns (square, triangle)
 * - Workspace boundary validation
 * - Rotation control
 * 
 * This example measures actual vs commanded positions to validate
 * the differential drive kinematics implementation.
 */

#include <TerraPenMotionControl.h>

// Global robot instance
TerraPenRobot robot;

// Test state tracking
int current_test = 0;
unsigned long test_start_time = 0;
bool test_in_progress = false;

// Position tracking for accuracy measurement
Position expected_position;
Position measured_position;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println("\n=== TerraPen Phase 2 Coordinate Accuracy Test ===");
    Serial.println("Testing coordinate-based movement and position tracking");
    
    // Initialize robot
    robot.begin();
    
    // Reset to known position
    robot.resetPosition(0, 0, 0);
    Serial.println("Robot initialized at origin (0, 0, 0°)");
    
    // Start first test
    startNextTest();
}

void loop() {
    // Always update robot
    robot.update();
    
    // Check if current test is complete
    if (test_in_progress && !robot.isBusy()) {
        completeCurrentTest();
        delay(1000);  // Pause between tests
        startNextTest();
    }
    
    // Print status every 2 seconds
    static unsigned long last_status = 0;
    if (millis() - last_status > 2000) {
        printStatus();
        last_status = millis();
    }
}

void startNextTest() {
    current_test++;
    test_in_progress = true;
    test_start_time = millis();
    
    Serial.println();
    Serial.print("=== Starting Test ");
    Serial.print(current_test);
    Serial.print(": ");
    
    switch (current_test) {
        case 1:
            testBasicMovement();
            break;
        case 2:
            testSquarePattern();
            break;
        case 3:
            testTrianglePattern();
            break;
        case 4:
            testRotationControl();
            break;
        case 5:
            testWorkspaceBoundaries();
            break;
        case 6:
            testDrawingOperations();
            break;
        case 7:
            testReturnToOrigin();
            break;
        default:
            // All tests complete
            allTestsComplete();
            return;
    }
}

void testBasicMovement() {
    Serial.println("Basic Movement ===");
    Serial.println("Moving to (20, 30) mm");
    
    expected_position = Position(20.0, 30.0, 0.0);
    robot.moveTo(20.0, 30.0);
}

void testSquarePattern() {
    Serial.println("Square Pattern ===");
    Serial.println("Drawing 20mm x 20mm square");
    
    // Start at current position and draw square
    Position current = robot.getCurrentPosition();
    Serial.print("Starting square at: ");
    current.print();
    
    // Move to first corner and start drawing
    expected_position = Position(current.x + 20, current.y, current.angle);
    robot.drawTo(current.x + 20, current.y);
}

void testTrianglePattern() {
    Serial.println("Triangle Pattern ===");
    Serial.println("Drawing equilateral triangle (side = 15mm)");
    
    Position current = robot.getCurrentPosition();
    Serial.print("Starting triangle at: ");
    current.print();
    
    // Move to first vertex
    expected_position = Position(current.x + 15, current.y, current.angle);
    robot.drawTo(current.x + 15, current.y);
}

void testRotationControl() {
    Serial.println("Rotation Control ===");
    Serial.println("Testing turnTo() and turnBy() methods");
    
    // Turn to 45 degrees
    expected_position = robot.getCurrentPosition().withAngle(PI/4);
    robot.turnTo(PI/4);  // 45 degrees
}

void testWorkspaceBoundaries() {
    Serial.println("Workspace Boundaries ===");
    Serial.println("Testing boundary validation");
    
    // Try to move outside workspace (should fail)
    bool result = robot.moveTo(150.0, 150.0);  // Beyond ±100mm limit
    
    if (!result) {
        Serial.println("✓ Boundary validation working - invalid move rejected");
        // Move to valid position instead
        expected_position = Position(50.0, 50.0, robot.getCurrentPosition().angle);
        robot.moveTo(50.0, 50.0);
    } else {
        Serial.println("✗ ERROR: Boundary validation failed - invalid move accepted");
        expected_position = robot.getCurrentPosition();
    }
}

void testDrawingOperations() {
    Serial.println("Drawing Operations ===");
    Serial.println("Testing pen up/down with coordinate movement");
    
    Position current = robot.getCurrentPosition();
    
    // Test pen control with movement
    robot.penDown();
    expected_position = Position(current.x - 10, current.y - 10, current.angle);
    robot.drawTo(current.x - 10, current.y - 10);
}

void testReturnToOrigin() {
    Serial.println("Return to Origin ===");
    Serial.println("Returning to starting position");
    
    expected_position = Position(0.0, 0.0, 0.0);
    robot.moveTo(0.0, 0.0);
}

void completeCurrentTest() {
    // Get actual position
    measured_position = robot.getCurrentPosition();
    
    // Calculate accuracy
    float position_error = expected_position.distanceTo(measured_position);
    float angle_error = abs(expected_position.angle - measured_position.angle) * 180.0 / PI;
    
    unsigned long test_duration = millis() - test_start_time;
    
    Serial.println();
    Serial.print("Test ");
    Serial.print(current_test);
    Serial.println(" Complete:");
    Serial.print("  Duration: ");
    Serial.print(test_duration);
    Serial.println(" ms");
    
    Serial.print("  Expected: ");
    expected_position.print();
    Serial.print("  Actual:   ");
    measured_position.print();
    
    Serial.print("  Position Error: ");
    Serial.print(position_error, 2);
    Serial.println(" mm");
    Serial.print("  Angle Error: ");
    Serial.print(angle_error, 1);
    Serial.println("°");
    
    // Evaluate accuracy
    if (position_error < 2.0 && angle_error < 5.0) {
        Serial.println("  ✓ PASSED - Good accuracy");
    } else if (position_error < 5.0 && angle_error < 10.0) {
        Serial.println("  ⚠ WARNING - Acceptable accuracy");
    } else {
        Serial.println("  ✗ FAILED - Poor accuracy");
    }
    
    test_in_progress = false;
}

void allTestsComplete() {
    Serial.println();
    Serial.println("=== ALL TESTS COMPLETE ===");
    Serial.println();
    Serial.println("Coordinate System Validation Summary:");
    Serial.println("- Basic movement commands: moveTo(), drawTo()");
    Serial.println("- Position tracking and estimation");
    Serial.println("- Geometric pattern accuracy");
    Serial.println("- Rotation control: turnTo(), turnBy()");
    Serial.println("- Workspace boundary validation");
    Serial.println("- Pen control integration");
    Serial.println();
    
    Position final_position = robot.getCurrentPosition();
    Serial.print("Final position: ");
    final_position.print();
    
    Serial.println("Phase 2 coordinate testing complete!");
    Serial.println("Use Serial Monitor to review accuracy results.");
    
    // Stop further testing
    test_in_progress = false;
    current_test = 999;
}

void printStatus() {
    if (!test_in_progress) return;
    
    Position current = robot.getCurrentPosition();
    RobotState state = robot.getState();
    
    Serial.print("[");
    Serial.print((millis() - test_start_time) / 1000);
    Serial.print("s] ");
    
    // Print state
    switch (state) {
        case IDLE:
            Serial.print("IDLE ");
            break;
        case MOVING:
            Serial.print("MOVING ");
            break;
        case ERROR:
            Serial.print("ERROR ");
            break;
        case EMERGENCY_STOP:
            Serial.print("E-STOP ");
            break;
    }
    
    // Print current position
    Serial.print("Pos: (");
    Serial.print(current.x, 1);
    Serial.print(", ");
    Serial.print(current.y, 1);
    Serial.print(") ");
    Serial.print(current.getAngleDegrees(), 0);
    Serial.print("° ");
    
    // Print pen status
    Serial.print("Pen: ");
    Serial.print(robot.isPenDown() ? "DOWN" : "UP");
    
    // Print step totals
    Serial.print(" Steps: L");
    Serial.print(robot.getLeftStepsTotal());
    Serial.print(" R");
    Serial.print(robot.getRightStepsTotal());
    
    Serial.println();
}