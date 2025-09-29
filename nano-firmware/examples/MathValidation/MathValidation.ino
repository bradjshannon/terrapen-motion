/**
 * TerraPen Motion Control Library - Comprehensive Mathematical Validation
 * 
 * This Arduino sketch validates all core mathematical algorithms that are:
 * - Complex and error-prone
 * - Critical for accurate robot movement
 * - Independent of physical hardware
 * 
 * Usage: Upload to Arduino Nano, open Serial Monitor at 9600 baud to see results
 */

#include <Arduino.h>
#include <Position.h>
#include <TerraPenConfig.h>

// Test counter and results
int total_tests = 0;
int passed_tests = 0;

void runTest(const char* test_name, bool condition) {
    total_tests++;
    Serial.print("Test: ");
    Serial.print(test_name);
    Serial.print(" ... ");
    if (condition) {
        passed_tests++;
        Serial.println("✓ PASS");
    } else {
        Serial.println("✗ FAIL");
    }
}

void setup() {
    Serial.begin(9600);
    while (!Serial) { delay(100); }
    
    Serial.println("=== TerraPen Motion Control - Mathematical Validation ===");
    Serial.println("Testing core algorithms without hardware dependency");
    Serial.println();
    
    // Initialize hardware parameters for testing
    TerraPenConfig::init();
    
    // === 1. Position Coordinate Mathematics ===
    Serial.println("--- Position Coordinate Mathematics ---");
    
    // Test 1: Distance calculation
    Position pos1(10.0, 10.0, 0.0);
    Position pos2(13.0, 14.0, 0.0);
    float dist = pos1.distanceTo(pos2);
    runTest("Distance calculation (3,4,5 triangle)", abs(dist - 5.0) < 0.001);
    
    // Test 2: Zero distance
    runTest("Zero distance to self", pos1.distanceTo(pos1) < 0.001);
    
    // Test 3: Angle calculation (East = 0°)
    float angle = pos1.angleTo(pos2);
    float expected_angle = atan2(4.0, 3.0) * 180.0 / PI;  // ~53.13°
    runTest("Angle calculation", abs(angle - expected_angle) < 0.1);
    
    // Test 4: Cardinal directions
    Position origin(0.0, 0.0, 0.0);
    Position north(0.0, 10.0, 0.0);
    Position east(10.0, 0.0, 0.0);
    Position south(0.0, -10.0, 0.0);
    Position west(-10.0, 0.0, 0.0);
    
    runTest("North angle (90°)", abs(origin.angleTo(north) - 90.0) < 0.1);
    runTest("East angle (0°)", abs(origin.angleTo(east) - 0.0) < 0.1);
    runTest("South angle (-90°)", abs(origin.angleTo(south) - (-90.0)) < 0.1);
    runTest("West angle (180°)", abs(abs(origin.angleTo(west)) - 180.0) < 0.1);
    
    // === 2. Angle Normalization ===
    Serial.println("--- Angle Normalization ---");
    
    runTest("Normalize 0°", abs(Position::normalizeAngle(0.0) - 0.0) < 0.001);
    runTest("Normalize 180°", abs(Position::normalizeAngle(180.0) - 180.0) < 0.001);
    runTest("Normalize -180°", abs(Position::normalizeAngle(-180.0) - (-180.0)) < 0.001);
    runTest("Normalize 360°", abs(Position::normalizeAngle(360.0) - 0.0) < 0.001);
    runTest("Normalize -360°", abs(Position::normalizeAngle(-360.0) - 0.0) < 0.001);
    runTest("Normalize 450°", abs(Position::normalizeAngle(450.0) - 90.0) < 0.001);
    runTest("Normalize -270°", abs(Position::normalizeAngle(-270.0) - 90.0) < 0.001);
    runTest("Normalize 720°", abs(Position::normalizeAngle(720.0) - 0.0) < 0.001);
    
    // === 3. Position Interpolation ===
    Serial.println("--- Position Interpolation ---");
    
    Position start(0.0, 0.0, 0.0);
    Position end(10.0, 10.0, 90.0);
    
    Position mid = Position::interpolate(start, end, 0.5);
    runTest("Interpolation midpoint X", abs(mid.x - 5.0) < 0.001);
    runTest("Interpolation midpoint Y", abs(mid.y - 5.0) < 0.001);
    runTest("Interpolation midpoint heading", abs(mid.heading - 45.0) < 0.001);
    
    Position quarter = Position::interpolate(start, end, 0.25);
    runTest("Interpolation quarter X", abs(quarter.x - 2.5) < 0.001);
    runTest("Interpolation quarter Y", abs(quarter.y - 2.5) < 0.001);
    
    runTest("Interpolation start (t=0)", Position::interpolate(start, end, 0.0).isEqual(start, 0.001));
    runTest("Interpolation end (t=1)", Position::interpolate(start, end, 1.0).isEqual(end, 0.001));
    
    // === 4. Differential Drive Kinematics ===
    Serial.println("--- Differential Drive Kinematics ---");
    
    // Test forward movement - both wheels same speed
    long left_steps, right_steps;
    TerraPenConfig::calculateWheelSteps(100.0, 0.0, left_steps, right_steps);  // 100mm forward, 0° turn
    runTest("Forward movement - equal wheel steps", left_steps == right_steps);
    runTest("Forward movement - positive steps", left_steps > 0 && right_steps > 0);
    
    // Test pure rotation - wheels opposite direction
    TerraPenConfig::calculateWheelSteps(0.0, 90.0, left_steps, right_steps);  // 0mm forward, 90° turn
    runTest("Pure rotation - opposite wheel directions", (left_steps > 0 && right_steps < 0) || (left_steps < 0 && right_steps > 0));
    runTest("Pure rotation - equal magnitude", abs(abs(left_steps) - abs(right_steps)) <= 1);  // Allow 1 step difference due to rounding
    
    // Test step calculations with real hardware specs
    // 28BYJ motor: 2048 steps/rev, 25mm wheel diameter = 78.54mm circumference
    float mm_per_step = (PI * 25.0) / 2048.0;  // ~0.0383mm per step
    float expected_steps_10mm = 10.0 / mm_per_step;
    
    TerraPenConfig::calculateWheelSteps(10.0, 0.0, left_steps, right_steps);
    runTest("10mm movement calculation", abs(left_steps - expected_steps_10mm) < 2);  // Allow 2 step tolerance
    
    // === 5. Coordinate-to-Movement Conversion ===  
    Serial.println("--- Coordinate-to-Movement Conversion ---");
    
    Position current(0.0, 0.0, 0.0);
    Position target(10.0, 0.0, 0.0);  // 10mm East
    
    float distance = current.distanceTo(target);
    float bearing = current.angleTo(target);
    float turn_angle = Position::normalizeAngle(bearing - current.heading);
    
    runTest("East movement distance", abs(distance - 10.0) < 0.001);
    runTest("East movement bearing", abs(bearing - 0.0) < 0.1);
    runTest("East movement turn angle", abs(turn_angle - 0.0) < 0.1);
    
    // Test diagonal movement
    Position diagonal_target(10.0, 10.0, 0.0);
    distance = current.distanceTo(diagonal_target);
    bearing = current.angleTo(diagonal_target);
    turn_angle = Position::normalizeAngle(bearing - current.heading);
    
    runTest("Diagonal movement distance", abs(distance - sqrt(200.0)) < 0.001);  // sqrt(10²+10²)
    runTest("Diagonal movement bearing", abs(bearing - 45.0) < 0.1);
    runTest("Diagonal movement turn angle", abs(turn_angle - 45.0) < 0.1);
    
    // === 6. Precision and Edge Cases ===
    Serial.println("--- Precision and Edge Cases ---");
    
    // Test very small movements
    Position tiny_target(0.1, 0.0, 0.0);  // 0.1mm
    distance = current.distanceTo(tiny_target);
    runTest("Tiny movement precision", abs(distance - 0.1) < 0.001);
    
    // Test angle wrap-around cases
    float wrapped_angle = Position::normalizeAngle(179.0 + 2.0);  // Should wrap to -179°
    runTest("Angle wrap-around 181°", abs(wrapped_angle - (-179.0)) < 0.1);
    
    wrapped_angle = Position::normalizeAngle(-179.0 - 2.0);  // Should wrap to 179°
    runTest("Angle wrap-around -181°", abs(wrapped_angle - 179.0) < 0.1);
    
    // Test turn optimization (shortest path)
    float turn_from_170_to_minus170 = Position::normalizeAngle(-170.0 - 170.0);  // Should be 20°, not 340°
    bool optimal_turn = abs(turn_from_170_to_minus170) <= 180.0;
    runTest("Turn optimization (shortest path)", optimal_turn);
    
    // === 7. Workspace Validation ===
    Serial.println("--- Workspace Validation ---");
    
    // Test position validity
    Position valid_pos(100.0, 100.0, 45.0);
    Position boundary_pos(0.0, 0.0, 0.0);
    
    runTest("Valid position creation", valid_pos.x == 100.0 && valid_pos.y == 100.0);
    runTest("Boundary position creation", boundary_pos.x == 0.0 && boundary_pos.y == 0.0);
    
    // Test position equality with tolerance
    Position approx_same(100.001, 100.001, 45.001);
    runTest("Position equality with tolerance", valid_pos.isEqual(approx_same, 0.01));
    runTest("Position inequality outside tolerance", !valid_pos.isEqual(approx_same, 0.0001));
    
    // === Results Summary ===
    Serial.println("\n" + String("=").substring(0,50));
    Serial.println("COMPREHENSIVE MATHEMATICAL VALIDATION COMPLETE");
    Serial.print("Tests passed: ");
    Serial.print(passed_tests);
    Serial.print(" / ");
    Serial.println(total_tests);
    
    float success_rate = (float)passed_tests / total_tests * 100.0;
    Serial.print("Success rate: ");
    Serial.print(success_rate, 1);
    Serial.println("%");
    
    if (passed_tests == total_tests) {
        Serial.println("\n🎉 ALL TESTS PASSED!");
        Serial.println("Core mathematical algorithms are working correctly.");
        Serial.println("The library is ready for coordinate-based robot control.");
    } else {
        Serial.println("\n⚠️  SOME TESTS FAILED!");
        Serial.println("Mathematical algorithms need attention before use.");
        Serial.print("Failed tests: ");
        Serial.println(total_tests - passed_tests);
    }
    
    Serial.println("\nThese tests validate the mathematical foundations");
    Serial.println("required for accurate differential drive robot movement.");
}

void loop() {
    // Tests run once in setup(), nothing in loop
    delay(10000);  // Long delay to prevent log spam
}