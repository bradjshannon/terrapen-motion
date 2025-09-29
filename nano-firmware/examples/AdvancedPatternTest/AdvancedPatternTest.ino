/**
 * AdvancedPatternTest.ino
 * 
 * Advanced coordinate system testing with complex patterns.
 * Tests accuracy with multi-segment paths and curve approximation.
 */

#include <TerraPenMotionControl.h>

TerraPenRobot robot;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println("\n=== Advanced Pattern Test ===");
    
    robot.begin();
    robot.resetPosition(0, 0, 0);
    
    // Draw a complex pattern to test coordinate accuracy
    drawComplexPattern();
}

void loop() {
    robot.update();
    
    // Print position every second
    static unsigned long last_print = 0;
    if (millis() - last_print > 1000) {
        Position pos = robot.getCurrentPosition();
        Serial.print("Position: ");
        pos.print();
        last_print = millis();
    }
}

void drawComplexPattern() {
    Serial.println("Drawing complex multi-segment pattern...");
    
    // Star pattern - tests accuracy over multiple moves
    robot.penUp();
    robot.moveTo(0, 20);      // Move to start
    robot.penDown();
    
    // Draw 5-pointed star
    robot.drawTo(15, 5);      // Right point
    robot.drawTo(-15, 12);    // Left upper
    robot.drawTo(15, 12);     // Right upper  
    robot.drawTo(-15, 5);     // Left point
    robot.drawTo(0, 20);      // Back to start
    
    robot.penUp();
    
    Serial.println("Star pattern complete");
    
    // Circle approximation - tests coordinated movement
    delay(2000);
    drawCircleApproximation();
}

void drawCircleApproximation() {
    Serial.println("Drawing circle approximation...");
    
    float radius = 15.0;
    int segments = 12;  // 12-sided polygon
    
    robot.moveTo(radius, 0);  // Move to start point
    robot.penDown();
    
    for (int i = 1; i <= segments; i++) {
        float angle = (2 * PI * i) / segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        robot.drawTo(x, y);
        
        // Print progress
        Serial.print("Segment ");
        Serial.print(i);
        Serial.print("/");
        Serial.println(segments);
    }
    
    robot.penUp();
    Serial.println("Circle complete");
}