/**
 * NVRAM Performance Data Manager
 * 
 * Manages performance metrics storage in Arduino EEPROM with:
 * - Circular buffer for efficient wear leveling
 * - ESP32 upload capability with confirmation tracking
 * - Automatic cleanup of uploaded data
 * - Space reservation for future features
 */

#ifndef NVRAM_MANAGER_H
#define NVRAM_MANAGER_H

#include <EEPROM.h>
#include "../PerformanceMonitor.h"
#include "../communication/ESP32Uploader.h"

// === EEPROM MEMORY LAYOUT ===
// Total EEPROM: 1024 bytes (Arduino Nano)
// Reserved for future: 256 bytes (25%)
// Available for logging: 768 bytes

struct EEPROMHeader {
    uint32_t magic_number;        // 0x54455252 ("TERR") - Format validation
    uint16_t format_version;      // Current: 1
    uint16_t write_cycles;        // Track EEPROM wear
    uint32_t total_records;       // Lifetime record count
    uint16_t buffer_start;        // Start of circular buffer
    uint16_t buffer_size;         // Size of circular buffer
    uint16_t next_write_pos;      // Next write position
    uint16_t oldest_record_pos;   // Position of oldest unuploaded record
    uint8_t reserved[12];         // Future expansion
} __attribute__((packed));

struct PerformanceRecord {
    uint32_t timestamp;           // millis() when recorded
    uint16_t cpu_usage_percent;   // 0-10000 (0.01% resolution)
    uint16_t free_memory_bytes;   // Available RAM
    uint16_t loop_time_us;        // Loop execution time
    uint16_t step_rate_sps;       // Steps per second
    uint8_t error_count;          // Errors since last record
    uint8_t flags;                // Bit 0: uploaded, Bit 1: confirmed, Bits 2-7: reserved
    uint16_t checksum;            // Data integrity check
} __attribute__((packed));

// Upload status bit flags (to minimize EEPROM wear)
#define UPLOAD_FLAG_SENT      0x01    // Bit 0: Data uploaded to ESP32
#define UPLOAD_FLAG_CONFIRMED 0x02    // Bit 1: Upload confirmed by ESP32
#define UPLOAD_FLAG_VALID     0x80    // Bit 7: Record contains valid data

class NVRAMManager {
private:
    static constexpr uint16_t HEADER_SIZE = sizeof(EEPROMHeader);
    static constexpr uint16_t RECORD_SIZE = sizeof(PerformanceRecord);
    static constexpr uint16_t EEPROM_SIZE = 1024;
    static constexpr uint16_t RESERVED_SIZE = 256;  // 25% reserved for future use
    static constexpr uint16_t BUFFER_START = HEADER_SIZE;
    static constexpr uint16_t BUFFER_SIZE = EEPROM_SIZE - RESERVED_SIZE - HEADER_SIZE;
    static constexpr uint16_t MAX_RECORDS = BUFFER_SIZE / RECORD_SIZE;
    
    EEPROMHeader header;
    ESP32Uploader* uploader;
    unsigned long last_upload_attempt;
    unsigned long upload_retry_interval;
    
    uint16_t calculateChecksum(const PerformanceRecord& record);
    bool validateRecord(const PerformanceRecord& record);
    bool isRecordUploaded(const PerformanceRecord& record);
    bool isRecordConfirmed(const PerformanceRecord& record);
    void markRecordUploaded(uint16_t address);
    void markRecordConfirmed(uint16_t address);
    void initializeEEPROM();
    void loadHeader();
    void saveHeader();
    void advanceWritePosition();
    void cleanupUploadedRecords();
    
public:
    NVRAMManager();
    
    // Initialization
    void begin(ESP32Uploader* uploader_instance = nullptr);
    bool isFormatted();
    void format();
    
    // Data storage
    bool storePerformanceData(const PerformanceMetrics& metrics);
    bool hasUnuploadedData();
    uint16_t getUnuploadedRecordCount();
    uint16_t getAvailableSpace();
    
    // Data retrieval
    bool getOldestRecord(PerformanceRecord& record);
    bool getRecordAt(uint16_t index, PerformanceRecord& record);
    bool getLatestRecords(PerformanceRecord* records, uint16_t count);
    
    // Upload management
    void setUploader(ESP32Uploader* uploader_instance);
    bool uploadPendingData();
    void confirmUploaded(uint32_t timestamp);
    void markRecordUploaded(uint32_t timestamp);
    
    // Maintenance
    void performMaintenance();
    float getEEPROMWearLevel();
    uint32_t getTotalRecordsStored();
    void getStorageStats(uint16_t& used, uint16_t& available, uint16_t& pending_upload);
    
    // Debug/diagnostics
    void printStorageStatus();
    bool verifyIntegrity();
};

// === UPLOAD STATUS CONSTANTS ===
enum UploadStatus : uint8_t {
    UPLOAD_PENDING = 0,    // Not yet uploaded
    UPLOAD_SENT = 1,       // Uploaded but not confirmed
    UPLOAD_CONFIRMED = 2   // Upload confirmed by ESP32
};

// === GLOBAL INSTANCE ===
extern NVRAMManager g_nvram_manager;

#endif // NVRAM_MANAGER_H