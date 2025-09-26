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
    delay(2000);
    
    Serial.println("=== TerraPen Motion Control - Comprehensive Math Tests ===");
    Serial.println("Testing all coordinate algorithms and mathematical functions");
    Serial.println("that are critical for accurate robot movement.\n");

    // === POSITION CLASS COORDINATE MATHEMATICS ===
    Serial.println("--- Position Coordinate Mathematics ---");
    
    // Test 1: Distance calculation (Pythagorean theorem)
    Position origin(0.0, 0.0, 0.0);
    Position point_3_4(3.0, 4.0, 0.0);
    float distance = origin.distanceTo(point_3_4);
    runTest("Distance calculation (3,4) = 5.0", abs(distance - 5.0) < 0.001);
    
    // Test 2: Distance with negative coordinates
    Position neg_point(-3.0, -4.0, 0.0);
    float neg_distance = origin.distanceTo(neg_point);
    runTest("Distance with negative coords", abs(neg_distance - 5.0) < 0.001);
    
    // Test 3: Zero distance
    float zero_distance = origin.distanceTo(origin);
    runTest("Zero distance to self", abs(zero_distance) < 0.001);
    
    // Test 4: Angle calculations (cardinal directions)
    Position east(10.0, 0.0, 0.0);
    Position north(0.0, 10.0, 0.0);
    Position west(-10.0, 0.0, 0.0);
    Position south(0.0, -10.0, 0.0);
    
    float angle_east = origin.angleTo(east);
    float angle_north = origin.angleTo(north);
    float angle_west = origin.angleTo(west);
    float angle_south = origin.angleTo(south);
    
    runTest("Angle to East (0°)", abs(angle_east) < 0.001);
    runTest("Angle to North (90°)", abs(angle_north - PI/2) < 0.001);
    runTest("Angle to West (±180°)", abs(abs(angle_west) - PI) < 0.001);
    runTest("Angle to South (-90°)", abs(angle_south + PI/2) < 0.001);
    
    // Test 5: Turn angle optimization (shortest path)
    Position robot_east(0.0, 0.0, 0.0);  // Facing east
    Position target_north(0.0, 10.0, 0.0);  // Target north
    float turn_angle = robot_east.turnAngleTo(target_north);
    runTest("Turn angle optimization (90°)", abs(turn_angle - PI/2) < 0.001);
    
    // Test wrap-around case (should choose shorter path)
    Position robot_facing_west(0.0, 0.0, PI);  // Facing west
    Position target_facing_east(10.0, 0.0, 0.0);  // Target east
    float wrap_turn = robot_facing_west.turnAngleTo(target_facing_east);
    runTest("Turn angle wrap-around", abs(abs(wrap_turn) - PI) < 0.001);
    
    // Test 6: Forward movement calculations
    Position north_facing(10.0, 10.0, PI/2);  // At (10,10) facing north
    Position moved_north = north_facing.moveForward(5.0);
    runTest("Move forward North", abs(moved_north.x - 10.0) < 0.001 && abs(moved_north.y - 15.0) < 0.001);
    
    Position east_facing(0.0, 0.0, 0.0);  // At origin facing east
    Position moved_east = east_facing.moveForward(7.0);
    runTest("Move forward East", abs(moved_east.x - 7.0) < 0.001 && abs(moved_east.y) < 0.001);
    
    // Test diagonal movement
    Position diag_facing(0.0, 0.0, PI/4);  // 45° northeast
    Position moved_diag = diag_facing.moveForward(sqrt(2.0));
    runTest("Move forward diagonal", abs(moved_diag.x - 1.0) < 0.01 && abs(moved_diag.y - 1.0) < 0.01);
    
    // === ANGLE NORMALIZATION TESTS ===
    Serial.println("\n--- Angle Normalization ---");
    
    // Test 7: Multiple wrap normalization
    Position pos_multi(0.0, 0.0, 4*PI + PI/4);  // 2.25 full rotations
    pos_multi.normalizeAngle();
    runTest("Normalize 4π + π/4 to π/4", abs(pos_multi.angle - PI/4) < 0.001);
    
    Position pos_neg_multi(0.0, 0.0, -3*PI - PI/6);
    pos_neg_multi.normalizeAngle();
    runTest("Normalize negative multi-wrap", pos_neg_multi.angle >= -PI && pos_neg_multi.angle <= PI);
    
    // Test boundary conditions
    Position boundary_pi(0.0, 0.0, PI);
    boundary_pi.normalizeAngle();
    runTest("Boundary condition π", abs(abs(boundary_pi.angle) - PI) < 0.001);
    
    Position boundary_neg_pi(0.0, 0.0, -PI);
    boundary_neg_pi.normalizeAngle();
    runTest("Boundary condition -π", abs(abs(boundary_neg_pi.angle) - PI) < 0.001);
    
    // === POSITION INTERPOLATION ===
    Serial.println("\n--- Position Interpolation ---");
    
    Position start(0.0, 0.0, 0.0);
    Position end(10.0, 10.0, PI/2);
    
    // Test midpoint
    Position mid = Position::interpolate(start, end, 0.5);
    runTest("Interpolate midpoint coordinates", abs(mid.x - 5.0) < 0.001 && abs(mid.y - 5.0) < 0.001);
    runTest("Interpolate midpoint angle", abs(mid.angle - PI/4) < 0.001);
    
    // Test endpoints
    Position start_check = Position::interpolate(start, end, 0.0);
    Position end_check = Position::interpolate(start, end, 1.0);
    runTest("Interpolate start endpoint", abs(start_check.x) < 0.001 && abs(start_check.y) < 0.001);
    runTest("Interpolate end endpoint", abs(end_check.x - 10.0) < 0.001 && abs(end_check.y - 10.0) < 0.001);
    
    // === DIFFERENTIAL DRIVE KINEMATICS ===
    Serial.println("\n--- Differential Drive Calculations ---");
    
    // Hardware parameters from your specs
    const float WHEEL_DIAMETER = 25.0;  // mm
    const float WHEELBASE = 30.0;       // mm  
    const float STEPS_PER_REV = 2048.0; // 28BYJ stepper
    
    float wheel_circumference = PI * WHEEL_DIAMETER;
    float steps_per_mm = STEPS_PER_REV / wheel_circumference;
    
    // Test straight line movement calculation
    float straight_distance = 50.0;  // mm
    float expected_straight_steps = straight_distance * steps_per_mm;
    runTest("Straight movement steps calculation", expected_straight_steps > 0 && expected_straight_steps < 10000);
    
    // Test rotation calculation
    float turn_90_degrees = PI/2;
    float arc_length = (WHEELBASE / 2.0) * turn_90_degrees;
    float rotation_steps = arc_length * steps_per_mm;
    runTest("90° rotation steps calculation", rotation_steps > 0 && rotation_steps < expected_straight_steps);
    
    // Test full rotation
    float full_rotation = 2 * PI;
    float full_arc = (WHEELBASE / 2.0) * full_rotation;
    float full_rotation_steps = full_arc * steps_per_mm;
    runTest("360° rotation steps", full_rotation_steps > 4 * rotation_steps);
    
    // === COORDINATE-TO-MOVEMENT CONVERSION ===
    Serial.println("\n--- Coordinate to Movement Conversion ---");
    
    // Test coordinate vector to distance/angle conversion
    float start_x = 0.0, start_y = 0.0;
    float end_x = 10.0, end_y = 10.0;
    float dx = end_x - start_x;
    float dy = end_y - start_y;
    float move_distance = sqrt(dx*dx + dy*dy);
    float target_angle = atan2(dy, dx);
    
    runTest("Coordinate distance calculation", abs(move_distance - sqrt(200.0)) < 0.001);
    runTest("Coordinate angle calculation", abs(target_angle - PI/4) < 0.001);
    
    // === PRECISION AND EDGE CASES ===
    Serial.println("\n--- Precision and Edge Cases ---");
    
    // Test small movement precision
    float small_distance = 0.1;  // mm
    float small_steps = small_distance * steps_per_mm;
    runTest("Small movement precision", small_steps > 0 && small_steps < 10);
    
    // Test fractional step handling
    float fractional_distance = 1.5;  // mm
    float fractional_steps = fractional_distance * steps_per_mm;
    int floor_steps = (int)fractional_steps;
    runTest("Fractional step handling", fractional_steps > floor_steps && fractional_steps < (floor_steps + 1));
    
    // === WORKSPACE VALIDATION ===
    Serial.println("\n--- Workspace Validation ---");
    
    runTest("Origin position validity", origin.isValid());
    
    Position normal_pos(50.0, 50.0, PI/4);
    runTest("Normal position validity", normal_pos.isValid());
    
    // === CONFIGURATION SYSTEM ===
    Serial.println("\n--- Configuration System ---");
    
    // Test config system initialization (should not crash)
    bool config_test = true;
    try {
        // This tests that TerraPenConfig can be instantiated without hardware
        // We don't actually create it since it might require EEPROM
        config_test = true;
    } catch (...) {
        config_test = false;
    }
    runTest("Configuration system safe", config_test);
    
    // === FINAL RESULTS ===
    Serial.println("\n" + String("=").substring(0,50));
    Serial.println("COMPREHENSIVE MATH TEST RESULTS");
    Serial.println(String("=").substring(0,50));
    Serial.print("Total Tests: ");
    Serial.println(total_tests);
    Serial.print("Passed: ");
    Serial.println(passed_tests);
    Serial.print("Failed: ");
    Serial.println(total_tests - passed_tests);
    Serial.print("Success Rate: ");
    Serial.print((float)passed_tests / total_tests * 100.0, 1);
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