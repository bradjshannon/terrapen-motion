/**
 * ESP32 Communication Interface
 * 
 * Handles data upload to ESP32 via Serial or other communication method.
 * Manages upload confirmation and retry logic.
 */

#ifndef ESP32_UPLOADER_H
#define ESP32_UPLOADER_H

#include <Arduino.h>

// Forward declaration
struct PerformanceRecord;

class ESP32Uploader {
private:
    HardwareSerial* serial_port;
    unsigned long last_heartbeat;
    unsigned long heartbeat_interval;
    bool esp32_connected;
    uint8_t retry_count;
    uint8_t max_retries;
    
    bool waitForAck(uint32_t timeout_ms);
    bool sendCommand(const String& command, uint32_t timeout_ms = 1000);
    void updateConnectionStatus();
    
public:
    ESP32Uploader();
    
    // Initialization
    void begin(HardwareSerial* port = &Serial, unsigned long baud_rate = 115200);
    void setHeartbeatInterval(unsigned long interval_ms);
    
    // Connection management
    bool isConnected();
    bool testConnection();
    void handleHeartbeat();
    
    // Data upload
    bool sendPerformanceData(const PerformanceRecord& record);
    bool sendBulkData(const PerformanceRecord* records, uint16_t count);
    bool sendErrorReport(uint8_t error_code, const String& context);
    bool sendStatusUpdate(const String& status);
    
    // Configuration
    void setMaxRetries(uint8_t retries);
    uint8_t getRetryCount();
    
    // Debug
    void printConnectionStatus();
};

// === PROTOCOL COMMANDS ===
// Commands sent to ESP32 for different data types
constexpr char CMD_PERFORMANCE_DATA[] = "PERF_DATA";
constexpr char CMD_ERROR_REPORT[] = "ERROR_RPT";
constexpr char CMD_STATUS_UPDATE[] = "STATUS";
constexpr char CMD_HEARTBEAT[] = "HEARTBEAT";
constexpr char CMD_TEST_CONNECTION[] = "TEST_CONN";

// Expected responses from ESP32
constexpr char RESP_ACK[] = "ACK";
constexpr char RESP_NACK[] = "NACK";
constexpr char RESP_READY[] = "READY";
constexpr char RESP_CONFIRM[] = "CONFIRM";

#endif // ESP32_UPLOADER_H