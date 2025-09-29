#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>

// Hardware pins
#define STATUS_LED 2
#define NANO_RESET_PIN 42
#define NANO_DTR_PIN 41
#define NANO_UART_RX 44
#define NANO_UART_TX 43

// WiFi credentials
const char* ssid = "TerraPen-Robot";
const char* password = "terrapen123";

// Objects
WebServer server(80);
HardwareSerial nanoSerial(2);
bool flashMode = false;
size_t totalFlashSize = 0;
size_t flashedBytes = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("TerraPen ESP32 Starting...");
    
    // Setup pins
    pinMode(STATUS_LED, OUTPUT);
    pinMode(NANO_RESET_PIN, OUTPUT);
    pinMode(NANO_DTR_PIN, OUTPUT);
    digitalWrite(NANO_RESET_PIN, HIGH);
    digitalWrite(NANO_DTR_PIN, HIGH);
    
    // Setup WiFi AP
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    digitalWrite(STATUS_LED, HIGH);
    
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
    
    // Setup OTA
    ArduinoOTA.setHostname("terrapen-esp32");
    ArduinoOTA.begin();
    
    // Setup UART for Arduino communication
    nanoSerial.begin(57600, SERIAL_8N1, NANO_UART_RX, NANO_UART_TX);
    
    // Setup web server
    server.on("/", []() {
        String html = "<html><head><title>TerraPen ESP32 Controller</title></head><body>";
        html += "<h1>TerraPen Robot Controller</h1>";
        html += "<h2>ESP32 OTA Update</h2>";
        html += "<p>Hostname: <strong>terrapen-esp32</strong></p>";
        html += "<p>Use Arduino IDE 'Upload via Network' or PlatformIO OTA</p>";
        html += "<h2>Arduino Nano Programming</h2>";
        html += "<form action='/flash' method='post' enctype='multipart/form-data'>";
        html += "<input type='file' name='firmware' accept='.hex' required>";
        html += "<button type='submit'>Flash Arduino</button>";
        html += "</form>";
        html += "<button onclick='fetch(\"/reset\", {method: \"POST\"})'>Reset Arduino</button>";
        html += "<div id='status'>Ready</div>";
        html += "<script>setInterval(() => fetch('/status').then(r => r.json()).then(d => ";
        html += "document.getElementById('status').innerHTML = ";
        html += "'Mode: ' + d.mode + '<br>Uptime: ' + d.uptime + 's'), 2000);</script>";
        html += "</body></html>";
        server.send(200, "text/html", html);
    });
    
    server.on("/status", []() {
        String json = "{";
        json += "\"mode\":\"" + String(flashMode ? "flashing" : "normal") + "\",";
        json += "\"uptime\":" + String(millis() / 1000) + ",";
        json += "\"flashProgress\":" + String(totalFlashSize > 0 ? (flashedBytes * 100 / totalFlashSize) : 0) + ",";
        json += "\"freeHeap\":" + String(ESP.getFreeHeap());
        json += "}";
        server.send(200, "application/json", json);
    });
    
    server.on("/flash", HTTP_POST, []() {
        if (flashMode) {
            server.send(202, "application/json", "{\"status\":\"flashing\",\"message\":\"Upload in progress\"}");
        } else {
            server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Arduino flash complete\"}");
        }
    }, []() {
        HTTPUpload& upload = server.upload();
        
        if (upload.status == UPLOAD_FILE_START) {
            Serial.printf("Arduino flash start: %s (%u bytes)\n", upload.filename.c_str(), upload.totalSize);
            flashMode = true;
            totalFlashSize = upload.totalSize;
            flashedBytes = 0;
            
            // Enter Arduino bootloader mode (DTR/Reset sequence)
            digitalWrite(NANO_DTR_PIN, LOW);
            digitalWrite(NANO_RESET_PIN, LOW);
            delay(100);
            digitalWrite(NANO_RESET_PIN, HIGH);
            delay(50);
            digitalWrite(NANO_DTR_PIN, HIGH);
            delay(100); // Give bootloader time to start
            
            digitalWrite(STATUS_LED, LOW); // Solid LED during flash
            
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            // Send data directly to Arduino bootloader
            nanoSerial.write(upload.buf, upload.currentSize);
            flashedBytes += upload.currentSize;
            
            // Progress feedback
            if (totalFlashSize > 0) {
                int progress = (flashedBytes * 100) / totalFlashSize;
                if (progress % 10 == 0) { // Log every 10%
                    Serial.printf("Upload progress: %d%%\n", progress);
                }
            }
            
        } else if (upload.status == UPLOAD_FILE_END) {
            Serial.println("Arduino flash complete - resetting Arduino");
            
            // Reset Arduino to start new firmware
            digitalWrite(NANO_RESET_PIN, LOW);
            delay(100);
            digitalWrite(NANO_RESET_PIN, HIGH);
            
            // Clean up
            flashMode = false;
            totalFlashSize = 0;
            flashedBytes = 0;
            digitalWrite(STATUS_LED, HIGH); // Ready indicator
            
        } else if (upload.status == UPLOAD_FILE_ABORTED) {
            Serial.println("Arduino flash aborted");
            flashMode = false;
            totalFlashSize = 0;
            flashedBytes = 0;
            digitalWrite(STATUS_LED, HIGH);
        }
    });
    
    // Direct HEX upload endpoint for PlatformIO integration
    server.on("/upload-hex", HTTP_POST, []() {
        server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"HEX upload complete\"}");
    }, []() {
        HTTPUpload& upload = server.upload();
        
        if (upload.status == UPLOAD_FILE_START) {
            Serial.printf("Direct HEX upload: %s\n", upload.filename.c_str());
            flashMode = true;
            
            // Enter bootloader mode
            digitalWrite(NANO_DTR_PIN, LOW);
            digitalWrite(NANO_RESET_PIN, LOW);
            delay(100);
            digitalWrite(NANO_RESET_PIN, HIGH);
            delay(50);
            digitalWrite(NANO_DTR_PIN, HIGH);
            delay(100);
            
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            nanoSerial.write(upload.buf, upload.currentSize);
            
        } else if (upload.status == UPLOAD_FILE_END) {
            Serial.println("HEX upload complete");
            digitalWrite(NANO_RESET_PIN, LOW);
            delay(100);
            digitalWrite(NANO_RESET_PIN, HIGH);
            flashMode = false;
        }
    });

    server.on("/reset", HTTP_POST, []() {
        // Reset Arduino
        digitalWrite(NANO_DTR_PIN, LOW);
        digitalWrite(NANO_RESET_PIN, LOW);
        delay(100);
        digitalWrite(NANO_RESET_PIN, HIGH);
        delay(50);
        digitalWrite(NANO_DTR_PIN, HIGH);
        server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Arduino reset\"}");
    });
    
    server.begin();
    Serial.println("Ready!");
}

void loop() {
    ArduinoOTA.handle();
    server.handleClient();
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
    delay(500);
}
