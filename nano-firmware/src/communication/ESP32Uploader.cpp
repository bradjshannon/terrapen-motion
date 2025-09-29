/**
 * ESP32 Communication Interface Implementation
 */

#include "ESP32Uploader.h"
#include "../storage/NVRAMManager.h"
#include "../ErrorSystem.h"

ESP32Uploader::ESP32Uploader() :
    serial_port(nullptr),
    last_heartbeat(0),
    heartbeat_interval(60000),  // 1 minute
    esp32_connected(false),
    retry_count(0),
    max_retries(3)
{
}

void ESP32Uploader::begin(HardwareSerial* port, unsigned long baud_rate) {
    serial_port = port;
    if (serial_port) {
        serial_port->begin(baud_rate);
        serial_port->setTimeout(1000);  // 1 second timeout for reads
        
        // Wait for serial to initialize
        delay(1000);
        
        // Test initial connection
        esp32_connected = testConnection();
    }
}

void ESP32Uploader::setHeartbeatInterval(unsigned long interval_ms) {
    heartbeat_interval = interval_ms;
}

bool ESP32Uploader::isConnected() {
    return esp32_connected && serial_port;
}

bool ESP32Uploader::testConnection() {
    if (!serial_port) return false;
    
    // Send test command and wait for response
    serial_port->println(CMD_TEST_CONNECTION);
    
    unsigned long start_time = millis();
    while (millis() - start_time < 2000) {  // 2 second timeout
        if (serial_port->available()) {
            String response = serial_port->readStringUntil('\n');
            response.trim();
            
            if (response == RESP_READY) {
                esp32_connected = true;
                last_heartbeat = millis();
                return true;
            }
        }
        delay(10);
    }
    
    esp32_connected = false;
    return false;
}

void ESP32Uploader::handleHeartbeat() {
    if (!serial_port) return;
    
    unsigned long current_time = millis();
    
    // Send heartbeat if interval has passed
    if (current_time - last_heartbeat > heartbeat_interval) {
        serial_port->println(CMD_HEARTBEAT);
        
        // Wait for heartbeat response (shorter timeout)
        if (waitForAck(500)) {
            esp32_connected = true;
            last_heartbeat = current_time;
        } else {
            esp32_connected = false;
        }
    }
}

bool ESP32Uploader::waitForAck(uint32_t timeout_ms) {
    if (!serial_port) return false;
    
    unsigned long start_time = millis();
    while (millis() - start_time < timeout_ms) {
        if (serial_port->available()) {
            String response = serial_port->readStringUntil('\n');
            response.trim();
            
            if (response == RESP_ACK || response == RESP_CONFIRM) {
                return true;
            } else if (response == RESP_NACK) {
                return false;
            }
        }
        delay(1);
    }
    
    return false;  // Timeout
}

bool ESP32Uploader::sendCommand(const String& command, uint32_t timeout_ms) {
    if (!serial_port) return false;
    
    for (uint8_t attempt = 0; attempt <= max_retries; attempt++) {
        serial_port->println(command);
        serial_port->flush();
        
        if (waitForAck(timeout_ms)) {
            retry_count = 0;  // Reset retry count on success
            return true;
        }
        
        retry_count++;
        delay(100 * (attempt + 1));  // Increasing delay between retries
    }
    
    return false;
}

bool ESP32Uploader::sendPerformanceData(const PerformanceRecord& record) {
    if (!isConnected()) {
        return false;
    }
    
    // Format performance data as JSON-like string for easy parsing
    String data_string = String(CMD_PERFORMANCE_DATA) + ":{";
    data_string += "\"ts\":" + String(record.timestamp) + ",";
    data_string += "\"cpu\":" + String(record.cpu_usage_percent) + ",";
    data_string += "\"mem\":" + String(record.free_memory_bytes) + ",";
    data_string += "\"loop\":" + String(record.loop_time_us) + ",";
    data_string += "\"steps\":" + String(record.step_rate_sps) + ",";
    data_string += "\"errors\":" + String(record.error_count);
    data_string += "}";
    
    bool success = sendCommand(data_string, 2000);
    
    if (success) {
        // If ESP32 confirms receipt, we can mark it as confirmed in NVRAM
        // This will be called by the NVRAM manager when it receives the confirmation
        // For now, just mark as sent
        return true;
    }
    
    return false;
}

bool ESP32Uploader::sendBulkData(const PerformanceRecord* records, uint16_t count) {
    if (!isConnected() || !records || count == 0) {
        return false;
    }
    
    // Send bulk data header
    String bulk_header = "BULK_DATA:{\"count\":" + String(count) + "}";
    if (!sendCommand(bulk_header, 1000)) {
        return false;
    }
    
    // Send each record
    uint16_t sent_count = 0;
    for (uint16_t i = 0; i < count; i++) {
        if (sendPerformanceData(records[i])) {
            sent_count++;
        } else {
            break;  // Stop on first failure
        }
        
        delay(10);  // Small delay between records
    }
    
    // Send bulk data footer
    String bulk_footer = "BULK_END:{\"sent\":" + String(sent_count) + "}";
    sendCommand(bulk_footer, 1000);
    
    return sent_count == count;
}

bool ESP32Uploader::sendErrorReport(uint8_t error_code, const String& context) {
    if (!isConnected()) {
        return false;
    }
    
    String error_data = String(CMD_ERROR_REPORT) + ":{";
    error_data += "\"code\":" + String(error_code) + ",";
    error_data += "\"context\":\"" + context + "\",";
    error_data += "\"timestamp\":" + String(millis());
    error_data += "}";
    
    return sendCommand(error_data, 1500);
}

bool ESP32Uploader::sendStatusUpdate(const String& status) {
    if (!isConnected()) {
        return false;
    }
    
    String status_data = String(CMD_STATUS_UPDATE) + ":{";
    status_data += "\"status\":\"" + status + "\",";
    status_data += "\"timestamp\":" + String(millis());
    status_data += "}";
    
    return sendCommand(status_data, 1000);
}

void ESP32Uploader::setMaxRetries(uint8_t retries) {
    max_retries = retries;
}

uint8_t ESP32Uploader::getRetryCount() {
    return retry_count;
}

void ESP32Uploader::printConnectionStatus() {
    Serial.println("=== ESP32 Connection Status ===");
    Serial.print("Connected: "); Serial.println(esp32_connected ? "YES" : "NO");
    Serial.print("Serial port: "); Serial.println(serial_port ? "Configured" : "Not configured");
    Serial.print("Last heartbeat: "); Serial.print(millis() - last_heartbeat); Serial.println(" ms ago");
    Serial.print("Retry count: "); Serial.println(retry_count);
    Serial.print("Max retries: "); Serial.println(max_retries);
}