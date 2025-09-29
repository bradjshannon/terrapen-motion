/**
 * TerraPen Motion Control - Arduino Nano Firmware
 * 
 * Main firmware for Arduino Nano controller
 * Handles real-time robot control, sensor processing, and ESP32 communication
 */

#ifndef MATH_VALIDATION_MODE

#include <Arduino.h>
#include "TerraPenConfig.h"
#include "robot/TerraPenRobot.h"
#include "ErrorSystem.h"
#include "PerformanceMonitor.h"
#include <ArduinoJson.h>

// Hardware configuration
TerraPenRobot robot;
PerformanceMonitor perf_monitor;

// Communication state
String inputBuffer = "";
unsigned long lastStatusUpdate = 0;
const unsigned long STATUS_UPDATE_INTERVAL = 1000; // 1 second

// Function declarations
void handleSerialCommands();
void processCommand(const String& command);
void sendAck();
void sendError(const String& errorMsg);
void sendPositionUpdate();
void sendStatusUpdate();

void setup() {
    // Initialize serial communication
    Serial.begin(57600);
    while (!Serial && millis() < 3000) { delay(10); } // Wait up to 3 seconds for serial
    
    Serial.println("TerraPen Nano Firmware Starting...");
    
    // Initialize robot
    robot.begin();
    Serial.println("✓ Robot initialized successfully");
    
    // Initialize performance monitor
    // perf_monitor will auto-initialize on first call
    
    Serial.println("TerraPen Nano Ready - Waiting for commands");
    sendStatusUpdate();
}

void loop() {
    // Update robot state machine
    robot.update();
    
    // Handle serial communication
    handleSerialCommands();
    
    // Send periodic status updates
    if (millis() - lastStatusUpdate >= STATUS_UPDATE_INTERVAL) {
        sendStatusUpdate();
        lastStatusUpdate = millis();
    }
    
    // Update performance monitoring
    // perf_monitor.update(); // TODO: Check API
    
    // Small delay to prevent overwhelming the serial buffer
    delay(10);
}

void handleSerialCommands() {
    while (Serial.available()) {
        char c = Serial.read();
        
        if (c == '\n' || c == '\r') {
            if (inputBuffer.length() > 0) {
                processCommand(inputBuffer);
                inputBuffer = "";
            }
        } else {
            inputBuffer += c;
        }
    }
}

void processCommand(const String& command) {
    // Parse JSON command
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, command);
    
    if (error) {
        sendError("Invalid JSON command");
        return;
    }
    
    if (!doc["cmd"].is<int>()) {
        sendError("Missing 'cmd' field");
        return;
    }
    
    int cmdId = doc["cmd"];
    
    switch (cmdId) {
        case 1: // MOVE_TO
            if (doc["x"].is<float>() && doc["y"].is<float>()) {
                float x = doc["x"];
                float y = doc["y"];
                bool penDown = doc["pen_down"].is<bool>() ? doc["pen_down"].as<bool>() : false;
                
                if (!penDown) {
                    robot.penUp(); // Pen up for movement
                }
                
                if (robot.moveTo(x, y)) {
                    sendAck();
                } else {
                    sendError("Move command failed");
                }
            } else {
                sendError("MOVE_TO requires x,y coordinates");
            }
            break;
            
        case 2: // DRAW_TO
            if (doc["x"].is<float>() && doc["y"].is<float>()) {
                float x = doc["x"];
                float y = doc["y"];
                
                if (robot.drawTo(x, y)) {
                    sendAck();
                } else {
                    sendError("Draw command failed");
                }
            } else {
                sendError("DRAW_TO requires x,y coordinates");
            }
            break;
            
        case 3: // SET_PEN
            if (doc["down"].is<bool>()) {
                bool down = doc["down"];
                if (down) {
                    robot.penDown();
                } else {
                    robot.penUp();
                }
                sendAck();
            } else {
                sendError("SET_PEN requires 'down' parameter");
            }
            break;
            
        case 4: // GET_POSITION
            sendPositionUpdate();
            break;
            
        case 5: // HOME
            // HOME command - move to origin
            if (robot.moveTo(0, 0)) {
                sendAck();
            } else {
                sendError("Home command failed");
            }
            break;
            
        case 6: // EMERGENCY_STOP
            robot.emergencyStop();
            sendAck();
            break;
            
        case 7: // GET_STATUS
            sendStatusUpdate();
            break;
            
        case 8: // CALIBRATE
            // TODO: Implement calibration routine
            sendError("Calibration not yet implemented");
            break;
            
        default:
            sendError("Unknown command ID: " + String(cmdId));
            break;
    }
}

void sendAck() {
    JsonDocument doc;
    doc["response"] = 128; // ACK
    doc["timestamp"] = millis();
    
    String response;
    serializeJson(doc, response);
    Serial.println(response);
}

void sendError(const String& errorMsg) {
    JsonDocument doc;
    doc["response"] = 129; // NACK
    doc["error_message"] = errorMsg;
    doc["timestamp"] = millis();
    
    String response;
    serializeJson(doc, response);
    Serial.println(response);
}

void sendPositionUpdate() {
    Position pos = robot.getCurrentPosition();
    
    JsonDocument doc;
    doc["response"] = 130; // POSITION
    doc["position"]["x"] = pos.x;
    doc["position"]["y"] = pos.y;
    doc["position"]["angle"] = pos.angle;
    doc["timestamp"] = millis();
    
    String response;
    serializeJson(doc, response);
    Serial.println(response);
}

void sendStatusUpdate() {
    JsonDocument doc;
    doc["response"] = 131; // STATUS
    
    // Robot state
    switch (robot.getState()) {
        case IDLE:
            doc["state"] = 0;
            break;
        case MOVING:
            doc["state"] = 1;
            break;
        case ERROR:
            doc["state"] = 2;
            break;
        case EMERGENCY_STOP:
            doc["state"] = 3;
            break;
    }
    
    doc["pen_down"] = robot.isPenDown();
    doc["timestamp"] = millis();
    
    // Add basic system info
    // TODO: Add performance data when API is available
    
    String response;
    serializeJson(doc, response);
    Serial.println(response);
}

#endif // MATH_VALIDATION_MODE