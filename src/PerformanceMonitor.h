#ifndef PERFORMANCE_MONITOR_H
#define PERFORMANCE_MONITOR_H

#include <Arduino.h>

/**
 * Performance Monitoring System
 * 
 * Provides real-time performance metrics for the robot system including:
 * - CPU utilization and timing analysis
 * - Memory usage tracking
 * - Hardware limit monitoring
 * - Update frequency and timing consistency
 * 
 * Designed to provide actionable insights for optimization and debugging.
 */

// === PERFORMANCE METRICS STRUCTURE ===

struct PerformanceMetrics {
    // Timing metrics (microseconds)
    unsigned long update_time_avg_us;      // Average update() execution time
    unsigned long update_time_max_us;      // Worst-case update() time
    unsigned long update_time_min_us;      // Best-case update() time
    unsigned long loop_time_avg_us;        // Average main loop time
    
    // Frequency metrics (Hz)
    float update_frequency_hz;             // Actual update() call frequency
    float target_frequency_hz;             // Target update frequency
    float loop_frequency_hz;               // Main loop frequency
    
    // CPU utilization (0.0 - 100.0%)
    float cpu_utilization_percent;         // % of time spent in update()
    float idle_time_percent;               // % of time available for other tasks
    
    // Memory metrics (bytes)
    int free_memory_bytes;                 // Available RAM
    int stack_usage_bytes;                 // Estimated stack usage
    int heap_usage_bytes;                  // Dynamic memory usage
    
    // Hardware stress indicators
    float motor_load_percent;              // Motor utilization %
    int missed_steps_total;                // Steps that couldn't execute on time
    int timing_violations;                 // Times when timing was violated
    
    // Statistical data
    unsigned long total_updates;           // Total update() calls since reset
    unsigned long total_runtime_ms;        // Total monitoring duration
    unsigned long last_reset_time_ms;      // When metrics were last reset
    
    PerformanceMetrics() { reset(); }
    
    void reset() {
        update_time_avg_us = 0;
        update_time_max_us = 0;
        update_time_min_us = ULONG_MAX;
        loop_time_avg_us = 0;
        update_frequency_hz = 0.0;
        target_frequency_hz = 1000.0;  // Default 1kHz target
        loop_frequency_hz = 0.0;
        cpu_utilization_percent = 0.0;
        idle_time_percent = 100.0;
        free_memory_bytes = 0;
        stack_usage_bytes = 0;
        heap_usage_bytes = 0;
        motor_load_percent = 0.0;
        missed_steps_total = 0;
        timing_violations = 0;
        total_updates = 0;
        total_runtime_ms = 0;
        last_reset_time_ms = millis();
    }
};

// === PERFORMANCE MONITOR CLASS ===

class PerformanceMonitor {
private:
    PerformanceMetrics metrics;
    
    // Timing measurement
    unsigned long update_start_time_us;
    unsigned long last_update_time_us;
    unsigned long loop_start_time_us;
    unsigned long last_loop_time_us;
    
    // Rolling averages
    static const int SAMPLE_HISTORY_SIZE = 50;
    unsigned long update_time_history[SAMPLE_HISTORY_SIZE];
    unsigned long loop_time_history[SAMPLE_HISTORY_SIZE];
    int history_index;
    int history_count;
    
    // Frequency calculation
    static const int FREQUENCY_SAMPLE_COUNT = 100;
    unsigned long frequency_sample_times[FREQUENCY_SAMPLE_COUNT];
    int frequency_sample_index;
    int frequency_sample_count;
    
    // Configuration
    bool monitoring_enabled;
    bool detailed_logging;
    unsigned long report_interval_ms;
    unsigned long last_report_time_ms;
    
    // Memory monitoring
    int baseline_free_memory;
    
public:
    PerformanceMonitor() : 
        history_index(0), history_count(0),
        frequency_sample_index(0), frequency_sample_count(0),
        monitoring_enabled(true), detailed_logging(false),
        report_interval_ms(5000), last_report_time_ms(0),
        baseline_free_memory(0) {
        
        // Initialize history arrays
        for (int i = 0; i < SAMPLE_HISTORY_SIZE; i++) {
            update_time_history[i] = 0;
            loop_time_history[i] = 0;
        }
        
        for (int i = 0; i < FREQUENCY_SAMPLE_COUNT; i++) {
            frequency_sample_times[i] = 0;
        }
        
        // Measure baseline memory
        baseline_free_memory = getFreeMemory();
    }
    
    // === TIMING MEASUREMENT ===
    
    /**
     * Call at the start of update() function
     */
    void startUpdate() {
        if (!monitoring_enabled) return;
        
        update_start_time_us = micros();
        
        // Calculate loop time since last update
        if (last_update_time_us > 0) {
            unsigned long loop_time = update_start_time_us - last_update_time_us;
            addLoopTimeSample(loop_time);
        }
        
        last_update_time_us = update_start_time_us;
    }
    
    /**
     * Call at the end of update() function
     */
    void endUpdate() {
        if (!monitoring_enabled) return;
        
        unsigned long end_time_us = micros();
        unsigned long update_duration = end_time_us - update_start_time_us;
        
        addUpdateTimeSample(update_duration);
        metrics.total_updates++;
        
        // Add frequency sample
        addFrequencySample(end_time_us);
        
        // Update derived metrics
        updateDerivedMetrics();
        
        // Periodic reporting
        if (millis() - last_report_time_ms > report_interval_ms) {
            if (detailed_logging) {
                printDetailedReport();
            }
            last_report_time_ms = millis();
        }
    }
    
    /**
     * Call at start of main loop iteration
     */
    void startLoop() {
        if (!monitoring_enabled) return;
        loop_start_time_us = micros();
    }
    
    /**
     * Call at end of main loop iteration
     */
    void endLoop() {
        if (!monitoring_enabled) return;
        // Loop timing is calculated in startUpdate()
    }
    
    // === HARDWARE MONITORING ===
    
    /**
     * Report a missed step (step that couldn't execute due to timing)
     */
    void reportMissedStep() {
        metrics.missed_steps_total++;
    }
    
    /**
     * Report a timing violation
     */
    void reportTimingViolation() {
        metrics.timing_violations++;
    }
    
    /**
     * Update motor load percentage based on current activity
     */
    void updateMotorLoad(float left_motor_load, float right_motor_load) {
        metrics.motor_load_percent = (left_motor_load + right_motor_load) / 2.0;
    }
    
    // === MEMORY MONITORING ===
    
    /**
     * Get current free memory (works on AVR platforms)
     */
    int getFreeMemory() {
        #if defined(__AVR__)
        extern char __heap_start, *__brkval;
        int v;
        return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
        #else
        return -1;  // Not implemented for this platform
        #endif
    }
    
    /**
     * Update memory metrics
     */
    void updateMemoryMetrics() {
        metrics.free_memory_bytes = getFreeMemory();
        if (baseline_free_memory > 0) {
            metrics.heap_usage_bytes = baseline_free_memory - metrics.free_memory_bytes;
        }
    }
    
    // === METRICS ACCESS ===
    
    /**
     * Get current performance metrics
     */
    PerformanceMetrics getMetrics() {
        updateMemoryMetrics();
        metrics.total_runtime_ms = millis() - metrics.last_reset_time_ms;
        return metrics;
    }
    
    /**
     * Reset all performance counters
     */
    void resetMetrics() {
        metrics.reset();
        history_index = 0;
        history_count = 0;
        frequency_sample_index = 0;
        frequency_sample_count = 0;
        baseline_free_memory = getFreeMemory();
    }
    
    // === CONFIGURATION ===
    
    void setMonitoringEnabled(bool enabled) { monitoring_enabled = enabled; }
    void setDetailedLogging(bool enabled) { detailed_logging = enabled; }
    void setReportInterval(unsigned long interval_ms) { report_interval_ms = interval_ms; }
    void setTargetFrequency(float target_hz) { metrics.target_frequency_hz = target_hz; }
    
    // === REPORTING ===
    
    /**
     * Print basic performance summary
     */
    void printSummary() {
        PerformanceMetrics m = getMetrics();
        
        Serial.println("=== Performance Summary ===");
        Serial.print("Update Frequency: ");
        Serial.print(m.update_frequency_hz, 1);
        Serial.print(" Hz (target: ");
        Serial.print(m.target_frequency_hz, 0);
        Serial.println(" Hz)");
        
        Serial.print("CPU Utilization: ");
        Serial.print(m.cpu_utilization_percent, 1);
        Serial.println("%");
        
        Serial.print("Update Time: ");
        Serial.print(m.update_time_avg_us);
        Serial.print(" μs avg, ");
        Serial.print(m.update_time_max_us);
        Serial.println(" μs max");
        
        if (m.free_memory_bytes >= 0) {
            Serial.print("Free Memory: ");
            Serial.print(m.free_memory_bytes);
            Serial.println(" bytes");
        }
        
        if (m.missed_steps_total > 0) {
            Serial.print("⚠️ Missed Steps: ");
            Serial.println(m.missed_steps_total);
        }
        
        if (m.timing_violations > 0) {
            Serial.print("⚠️ Timing Violations: ");
            Serial.println(m.timing_violations);
        }
        
        Serial.println("===========================");
    }
    
    /**
     * Print detailed performance report
     */
    void printDetailedReport() {
        PerformanceMetrics m = getMetrics();
        
        Serial.println("\n=== Detailed Performance Report ===");
        
        // Timing metrics
        Serial.println("TIMING:");
        Serial.print("  Update: ");
        Serial.print(m.update_time_avg_us);
        Serial.print("μs avg, ");
        Serial.print(m.update_time_min_us);
        Serial.print("-");
        Serial.print(m.update_time_max_us);
        Serial.println("μs range");
        
        Serial.print("  Loop: ");
        Serial.print(m.loop_time_avg_us);
        Serial.println("μs avg");
        
        // Frequency metrics
        Serial.println("FREQUENCY:");
        Serial.print("  Update: ");
        Serial.print(m.update_frequency_hz, 1);
        Serial.print(" Hz (");
        Serial.print((m.update_frequency_hz / m.target_frequency_hz) * 100.0, 1);
        Serial.println("% of target)");
        
        Serial.print("  Loop: ");
        Serial.print(m.loop_frequency_hz, 1);
        Serial.println(" Hz");
        
        // CPU utilization
        Serial.println("CPU:");
        Serial.print("  Utilization: ");
        Serial.print(m.cpu_utilization_percent, 1);
        Serial.println("%");
        Serial.print("  Idle Time: ");
        Serial.print(m.idle_time_percent, 1);
        Serial.println("%");
        
        // Memory
        if (m.free_memory_bytes >= 0) {
            Serial.println("MEMORY:");
            Serial.print("  Free: ");
            Serial.print(m.free_memory_bytes);
            Serial.println(" bytes");
            Serial.print("  Heap Usage: ");
            Serial.print(m.heap_usage_bytes);
            Serial.println(" bytes");
        }
        
        // Hardware stress
        Serial.println("HARDWARE:");
        Serial.print("  Motor Load: ");
        Serial.print(m.motor_load_percent, 1);
        Serial.println("%");
        Serial.print("  Missed Steps: ");
        Serial.println(m.missed_steps_total);
        Serial.print("  Timing Violations: ");
        Serial.println(m.timing_violations);
        
        // Statistics
        Serial.println("STATISTICS:");
        Serial.print("  Total Updates: ");
        Serial.println(m.total_updates);
        Serial.print("  Runtime: ");
        Serial.print(m.total_runtime_ms / 1000.0, 1);
        Serial.println(" seconds");
        
        Serial.println("================================\n");
    }
    
    /**
     * Get performance metrics as JSON string
     */
    String getMetricsJson() {
        PerformanceMetrics m = getMetrics();
        
        String json = "{";
        json += "\"update_freq_hz\":" + String(m.update_frequency_hz) + ",";
        json += "\"cpu_utilization\":" + String(m.cpu_utilization_percent) + ",";
        json += "\"update_time_avg_us\":" + String(m.update_time_avg_us) + ",";
        json += "\"update_time_max_us\":" + String(m.update_time_max_us) + ",";
        json += "\"free_memory\":" + String(m.free_memory_bytes) + ",";
        json += "\"missed_steps\":" + String(m.missed_steps_total) + ",";
        json += "\"timing_violations\":" + String(m.timing_violations) + ",";
        json += "\"motor_load\":" + String(m.motor_load_percent) + ",";
        json += "\"total_updates\":" + String(m.total_updates) + ",";
        json += "\"runtime_ms\":" + String(m.total_runtime_ms);
        json += "}";
        
        return json;
    }
    
private:
    void addUpdateTimeSample(unsigned long time_us) {
        // Add to history
        update_time_history[history_index] = time_us;
        history_index = (history_index + 1) % SAMPLE_HISTORY_SIZE;
        if (history_count < SAMPLE_HISTORY_SIZE) history_count++;
        
        // Update min/max
        if (time_us > metrics.update_time_max_us) {
            metrics.update_time_max_us = time_us;
        }
        if (time_us < metrics.update_time_min_us) {
            metrics.update_time_min_us = time_us;
        }
        
        // Calculate average
        unsigned long total = 0;
        for (int i = 0; i < history_count; i++) {
            total += update_time_history[i];
        }
        metrics.update_time_avg_us = total / history_count;
    }
    
    void addLoopTimeSample(unsigned long time_us) {
        loop_time_history[history_index] = time_us;
        
        // Calculate loop time average
        unsigned long total = 0;
        for (int i = 0; i < history_count; i++) {
            total += loop_time_history[i];
        }
        if (history_count > 0) {
            metrics.loop_time_avg_us = total / history_count;
        }
    }
    
    void addFrequencySample(unsigned long time_us) {
        frequency_sample_times[frequency_sample_index] = time_us;
        frequency_sample_index = (frequency_sample_index + 1) % FREQUENCY_SAMPLE_COUNT;
        if (frequency_sample_count < FREQUENCY_SAMPLE_COUNT) frequency_sample_count++;
        
        // Calculate frequency if we have enough samples
        if (frequency_sample_count >= 10) {
            unsigned long oldest_time = frequency_sample_times[frequency_sample_index];
            unsigned long duration_us = time_us - oldest_time;
            
            if (duration_us > 0) {
                metrics.update_frequency_hz = (frequency_sample_count - 1) * 1000000.0 / duration_us;
            }
        }
    }
    
    void updateDerivedMetrics() {
        // Calculate CPU utilization
        if (metrics.loop_time_avg_us > 0) {
            metrics.cpu_utilization_percent = 
                (float)metrics.update_time_avg_us / metrics.loop_time_avg_us * 100.0;
            metrics.idle_time_percent = 100.0 - metrics.cpu_utilization_percent;
        }
        
        // Calculate loop frequency
        if (metrics.loop_time_avg_us > 0) {
            metrics.loop_frequency_hz = 1000000.0 / metrics.loop_time_avg_us;
        }
    }
};

// Global performance monitor instance
extern PerformanceMonitor g_performance_monitor;

// === CONVENIENCE MACROS ===

#define PERF_START_UPDATE() g_performance_monitor.startUpdate()
#define PERF_END_UPDATE() g_performance_monitor.endUpdate()
#define PERF_START_LOOP() g_performance_monitor.startLoop()
#define PERF_END_LOOP() g_performance_monitor.endLoop()
#define PERF_REPORT_MISSED_STEP() g_performance_monitor.reportMissedStep()
#define PERF_REPORT_TIMING_VIOLATION() g_performance_monitor.reportTimingViolation()
#define PERF_PRINT_SUMMARY() g_performance_monitor.printSummary()

#endif // PERFORMANCE_MONITOR_H