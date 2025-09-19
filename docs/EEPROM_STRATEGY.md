/**
 * Arduino EEPROM Performance Storage Layout
 * 
 * Optimized for 1KB EEPROM with wear leveling
 */

// === EEPROM MEMORY MAP ===
// Address 0-15: Configuration header
// Address 16-31: Error history (recent errors)  
// Address 32-63: Performance statistics
// Address 64-127: Test results cache
// Address 128-1023: Rotating log buffer

struct EEPROMLayout {
    // Header (16 bytes)
    uint32_t magic_number;     // 0xTERRAP01 - validates EEPROM format
    uint16_t write_count;      // Track EEPROM wear
    uint16_t version;          // Data format version
    uint64_t last_update;      // Timestamp of last write
    
    // Error History (16 bytes) - Last 8 errors
    struct {
        uint8_t error_code;
        uint8_t context_id;
    } recent_errors[8];
    
    // Performance Stats (32 bytes)
    struct {
        uint32_t total_runtime_hours;
        uint16_t successful_posts;
        uint16_t failed_posts;
        uint16_t emergency_stops;
        uint16_t max_cpu_usage;
        uint16_t avg_cpu_usage;
        uint16_t total_movements;
        uint32_t total_steps;
        uint16_t max_step_rate;
        uint16_t servo_operations;
    } performance_stats;
    
    // Test Results Cache (64 bytes) - Recent test outcomes
    struct {
        uint8_t test_id;
        uint8_t result;           // PASS/FAIL/SKIP
        uint16_t execution_time;  // milliseconds
    } test_cache[16];
};

// === EEPROM WEAR LEVELING ===
// Rotate write locations for frequently updated data
class EEPROMManager {
    private:
        uint16_t log_write_position = 128;  // Start of rotating buffer
        
    public:
        void writePerformanceData(const PerformanceMetrics& metrics) {
            // Update counters in fixed location (less frequent)
            updatePerformanceStats(metrics);
            
            // Write detailed log entry to rotating buffer
            writeToRotatingLog(metrics);
            
            // Advance write position with wraparound
            log_write_position += sizeof(LogEntry);
            if (log_write_position > 1000) log_write_position = 128;
        }
};

// === USAGE PATTERN ===
void setup() {
    EEPROMManager storage;
    
    // Load previous performance data
    storage.loadPerformanceStats();
    
    // Run POST and cache results
    bool post_result = runQuickPost();
    storage.cacheTestResults(post_result);
    
    // Start performance monitoring
    performance_monitor.begin();
}

void loop() {
    // Every 1000 loops (~10 seconds), save performance data
    if (loop_counter % 1000 == 0) {
        storage.writePerformanceData(performance_monitor.getMetrics());
    }
}