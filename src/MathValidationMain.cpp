/**
 * TerraPen Motion Control Library - Mathematical Validation Test Program
 * This is a simple test executable that can be run to validate the core algorithms
 */

#ifdef MATH_VALIDATION_MODE

#include <Arduino.h>
#include "Position.h"

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
    Serial.println("Testing core position algorithms without hardware dependency");
    Serial.println();
    
    // === 1. Position Coordinate Mathematics ===
    Serial.println("--- Position Coordinate Mathematics ---");
    
    // Test 1: Distance calculation (3,4,5 triangle)
    Position pos1(0.0, 0.0, 0.0);
    Position pos2(3.0, 4.0, 0.0);
    float dist = pos1.distanceTo(pos2);
    runTest("Distance calculation (3,4,5 triangle)", abs(dist - 5.0) < 0.001);
    
    // Test 2: Zero distance
    runTest("Zero distance to self", pos1.distanceTo(pos1) < 0.001);
    
    // Test 3: Cardinal directions  
    Position origin(0.0, 0.0, 0.0);
    Position north(0.0, 10.0, 0.0);
    Position east(10.0, 0.0, 0.0);
    
    float north_angle = origin.angleTo(north);
    float east_angle = origin.angleTo(east);
    
    runTest("North direction angle", abs(north_angle - 90.0) < 0.1);
    runTest("East direction angle", abs(east_angle - 0.0) < 0.1);
    
    // === 2. Position Creation and Equality ===
    Serial.println("--- Position Creation and Equality ---");
    
    Position test_pos(100.0, 200.0, PI/4);
    runTest("Position creation X", abs(test_pos.x - 100.0) < 0.001);
    runTest("Position creation Y", abs(test_pos.y - 200.0) < 0.001);
    runTest("Position creation angle", abs(test_pos.angle - PI/4) < 0.001);
    
    // Test position component access
    Position same_pos(100.0, 200.0, PI/4);
    bool positions_equal = (abs(test_pos.x - same_pos.x) < 0.001 && 
                           abs(test_pos.y - same_pos.y) < 0.001 &&
                           abs(test_pos.angle - same_pos.angle) < 0.001);
    runTest("Position equality check", positions_equal);
    
    // === 3. Position Interpolation ===
    Serial.println("--- Position Interpolation ---");
    
    Position start(0.0, 0.0, 0.0);
    Position end(10.0, 10.0, PI/2);
    
    Position mid = Position::interpolate(start, end, 0.5);
    runTest("Interpolation midpoint X", abs(mid.x - 5.0) < 0.001);
    runTest("Interpolation midpoint Y", abs(mid.y - 5.0) < 0.001);
    
    Position quarter = Position::interpolate(start, end, 0.25);
    runTest("Interpolation quarter X", abs(quarter.x - 2.5) < 0.001);
    runTest("Interpolation quarter Y", abs(quarter.y - 2.5) < 0.001);
    
    // === 4. Angle Normalization ===
    Serial.println("--- Angle Normalization ---");
    
    // Test instance angle normalization
    Position angle_test(0.0, 0.0, 3*PI);  // 540 degrees
    angle_test.normalizeAngle();
    runTest("Normalize 3π to π", abs(angle_test.angle - PI) < 0.001);
    
    Position angle_test2(0.0, 0.0, -3*PI);  // -540 degrees
    angle_test2.normalizeAngle();
    runTest("Normalize -3π to -π", abs(angle_test2.angle - (-PI)) < 0.001);
    
    // === 5. Polar Coordinate Creation ===
    Serial.println("--- Polar Coordinates ---");
    
    Position polar_pos = Position::fromPolar(10.0, PI/4);  // 10mm at 45 degrees
    runTest("Polar creation X component", abs(polar_pos.x - 10.0*sin(PI/4)) < 0.001);
    runTest("Polar creation Y component", abs(polar_pos.y - 10.0*cos(PI/4)) < 0.001);
    
    // === 6. Movement Calculations ===  
    Serial.println("--- Movement Calculations ---");
    
    Position current(0.0, 0.0, 0.0);
    Position target(10.0, 0.0, 0.0);  // 10mm East
    
    float distance = current.distanceTo(target);
    float bearing = current.angleTo(target);
    
    runTest("East movement distance", abs(distance - 10.0) < 0.001);
    runTest("East movement bearing (0°)", abs(bearing - 0.0) < 0.1);
    
    // Test diagonal movement
    Position diagonal_target(10.0, 10.0, 0.0);
    distance = current.distanceTo(diagonal_target);
    bearing = current.angleTo(diagonal_target);
    
    float expected_diagonal_dist = sqrt(10.0*10.0 + 10.0*10.0);
    runTest("Diagonal movement distance", abs(distance - expected_diagonal_dist) < 0.001);
    runTest("Diagonal movement bearing (45°)", abs(bearing - 45.0) < 0.1);
    
    // === 7. Edge Cases and Precision ===
    Serial.println("--- Edge Cases and Precision ---");
    
    // Test very small movements
    Position tiny_target(0.1, 0.0, 0.0);  // 0.1mm
    distance = current.distanceTo(tiny_target);
    runTest("Tiny movement precision", abs(distance - 0.1) < 0.001);
    
    // Test position equality with tolerance
    Position approx_same(100.001, 200.001, PI/4 + 0.001);
    Position reference(100.0, 200.0, PI/4);
    
    bool within_tolerance = (abs(reference.x - approx_same.x) < 0.01 &&
                            abs(reference.y - approx_same.y) < 0.01 &&
                            abs(reference.angle - approx_same.angle) < 0.01);
    runTest("Position equality with tolerance", within_tolerance);
    
    bool outside_tolerance = (abs(reference.x - approx_same.x) < 0.0001 &&
                             abs(reference.y - approx_same.y) < 0.0001 &&
                             abs(reference.angle - approx_same.angle) < 0.0001);
    runTest("Position inequality outside tolerance", !outside_tolerance);
    
    // === Results Summary ===
    Serial.println("\n" + String("=").substring(0,50));
    Serial.println("MATHEMATICAL VALIDATION COMPLETE");
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
        Serial.println("Position coordinate system is ready for robot control.");
    } else {
        Serial.println("\n⚠️  SOME TESTS FAILED!");
        Serial.println("Mathematical algorithms need attention before use.");
        Serial.print("Failed tests: ");
        Serial.println(total_tests - passed_tests);
    }
    
    Serial.println("\nValidated: coordinate math, interpolation, angle handling, polar coordinates");
    Serial.println("These are the mathematical foundations for robot movement.");
}

void loop() {
    // Tests run once in setup(), nothing in loop
    delay(10000);  // Long delay to prevent log spam
}

#endif  // MATH_VALIDATION_MODE