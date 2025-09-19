# Adaptive Performance Monitoring Strategy

## Overview
Smart performance logging that minimizes EEPROM wear while capturing detailed data only when needed.

## Strategy Design

### Normal Operation (Baseline Mode)
- **Logging Interval**: Every 10 minutes (600,000ms)
- **Data Type**: Broad performance patterns only
- **EEPROM Writes**: Minimal - every 10 minutes
- **Memory Usage**: 3-sample window (lightweight)

### Anomaly Detection Mode
- **Trigger Conditions**:
  - Loop timing > 5ms (normally <1ms)
  - Update frequency < 90Hz (normally 100Hz+)
  - CPU usage > 80%
  - Step timing variance > 50μs
- **Logging Interval**: Every 5 seconds (detailed capture)
- **Duration**: 30 seconds of detailed logging
- **Cooldown**: Requires 3 normal readings to exit

### EEPROM Wear Protection
- **Normal Operation**: 1 write every 10 minutes = 144 writes/day
- **Maximum Anomaly Rate**: 12 anomaly records/hour (5-min minimum spacing)
- **Total Daily Writes**: ~432 maximum (well within EEPROM 100,000 cycle limit)

## Data Coverage Analysis

### With 30 NVRAM Records:
- **Baseline Coverage**: 30 × 10 minutes = 5 hours of broad patterns
- **Anomaly Coverage**: 30 × 5 seconds = 2.5 minutes of detailed data
- **Mixed Coverage**: Typical usage provides 3-4 hours of operational insight

### Data Types Stored:

#### Baseline Records (Normal Operation)
```cpp
struct BaselineRecord {
    uint32_t timestamp;           // 4 bytes
    uint8_t avg_cpu_percent;      // 1 byte - averaged over 10 minutes
    uint16_t avg_frequency_hz;    // 2 bytes - average update rate
    uint16_t avg_loop_time_us;    // 2 bytes - average timing
    uint8_t step_performance;     // 1 byte - stepper health score
    uint8_t flags;                // 1 byte - status flags
    // Total: 11 bytes per baseline record
};
```

#### Anomaly Records (Detailed Capture)
```cpp
struct AnomalyRecord {
    uint32_t timestamp;           // 4 bytes
    uint16_t trigger_reason;      // 2 bytes - what caused anomaly
    uint16_t peak_loop_time_us;   // 2 bytes - worst timing during anomaly
    uint16_t min_frequency_hz;    // 2 bytes - lowest frequency
    uint8_t peak_cpu_percent;     // 1 byte - highest CPU usage
    uint16_t step_timing_variance;// 2 bytes - stepper irregularity
    uint8_t recovery_time_s;      // 1 byte - time to return to normal
    uint8_t flags;                // 1 byte - detailed status
    // Total: 15 bytes per anomaly record
};
```

## Benefits

### 1. EEPROM Longevity
- **Baseline**: 144 writes/day vs 1,440 writes/day (10x reduction)
- **Lifecycle**: 690+ days of continuous operation at baseline rate
- **Wear Protection**: Built-in limits prevent anomaly spam

### 2. Meaningful Data
- **Pattern Detection**: 5-hour baseline coverage shows operational trends
- **Issue Diagnosis**: Detailed anomaly data captures exactly when/why problems occur
- **Resource Efficiency**: No wasted storage on redundant normal operation data

### 3. Adaptive Intelligence
- **Self-Tuning**: System learns normal vs abnormal performance patterns
- **Context Awareness**: Detailed logging only when it matters
- **Predictive Capability**: Pattern analysis can predict issues before they become critical

## Implementation Notes

### State Machine
```
BASELINE_MODE → (anomaly detected) → ANOMALY_MODE → (30s timeout) → COOLDOWN_MODE → (3 normal cycles) → BASELINE_MODE
```

### ESP32 Integration
- **Upload Frequency**: ESP32 can upload every 5-10 minutes over UART
- **Data Prioritization**: Anomaly records uploaded immediately
- **Baseline Batching**: Multiple baseline records uploaded together

### Memory Optimization
- **Baseline Mode**: 3 × 11 bytes = 33 bytes RAM usage
- **Anomaly Mode**: 15 × 15 bytes = 225 bytes RAM usage (temporary)
- **Adaptive Buffer**: Switches size based on mode

This strategy provides the broad operational insights you need while capturing detailed data precisely when performance deviates from normal patterns, all while minimizing EEPROM wear for long-term reliability.