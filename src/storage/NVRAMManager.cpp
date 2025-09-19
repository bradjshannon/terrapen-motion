/**
 * NVRAM Performance Data Manager Implementation
 */

#include "NVRAMManager.h"
#include "../ErrorSystem.h"

// Global instance
NVRAMManager g_nvram_manager;

NVRAMManager::NVRAMManager() : 
    uploader(nullptr),
    last_upload_attempt(0),
    upload_retry_interval(30000)  // 30 seconds between upload attempts
{
    memset(&header, 0, sizeof(header));
}

void NVRAMManager::begin(ESP32Uploader* uploader_instance) {
    uploader = uploader_instance;
    
    if (!isFormatted()) {
        format();
    } else {
        loadHeader();
    }
}

bool NVRAMManager::isFormatted() {
    EEPROMHeader temp_header;
    EEPROM.get(0, temp_header);
    return (temp_header.magic_number == 0x54455252 && 
            temp_header.format_version == 1);
}

void NVRAMManager::format() {
    // Initialize header
    header.magic_number = 0x54455252;  // "TERR"
    header.format_version = 1;
    header.write_cycles = 0;
    header.total_records = 0;
    header.buffer_start = BUFFER_START;
    header.buffer_size = BUFFER_SIZE;
    header.next_write_pos = BUFFER_START;
    header.oldest_record_pos = BUFFER_START;
    
    saveHeader();
    
    // Clear the data area
    for (uint16_t addr = BUFFER_START; addr < BUFFER_START + BUFFER_SIZE; addr++) {
        EEPROM.write(addr, 0xFF);
    }
}

void NVRAMManager::loadHeader() {
    EEPROM.get(0, header);
}

void NVRAMManager::saveHeader() {
    EEPROM.put(0, header);
    header.write_cycles++;
}

uint16_t NVRAMManager::calculateChecksum(const PerformanceRecord& record) {
    uint16_t checksum = 0;
    const uint8_t* data = (const uint8_t*)&record;
    
    // Calculate checksum for all fields except checksum itself
    for (uint16_t i = 0; i < sizeof(PerformanceRecord) - sizeof(uint16_t); i++) {
        checksum += data[i];
    }
    
    return checksum;
}

bool NVRAMManager::validateRecord(const PerformanceRecord& record) {
    return calculateChecksum(record) == record.checksum;
}

bool NVRAMManager::storePerformanceData(const PerformanceMetrics& metrics) {
    PerformanceRecord record;
    
    // Convert metrics to record format
    record.timestamp = millis();
    record.cpu_usage_percent = metrics.cpu_utilization_percent * 100;  // 0.01% resolution
    record.free_memory_bytes = metrics.free_memory_bytes;
    record.loop_time_us = metrics.loop_time_avg_us;
    record.step_rate_sps = metrics.motor_load_percent;  // Use motor load as proxy for step rate
    record.error_count = metrics.timing_violations;
    record.flags = UPLOAD_FLAG_VALID;  // Mark as valid, not uploaded yet
    record.checksum = calculateChecksum(record);
    
    // Store record at next write position
    EEPROM.put(header.next_write_pos, record);
    
    // Update header
    header.total_records++;
    advanceWritePosition();
    saveHeader();
    
    // Check if we need to upload data
    if (uploader && hasUnuploadedData() && 
        (millis() - last_upload_attempt > upload_retry_interval)) {
        uploadPendingData();
    }
    
    return true;
}

bool NVRAMManager::isRecordUploaded(const PerformanceRecord& record) {
    return (record.flags & UPLOAD_FLAG_SENT) != 0;
}

bool NVRAMManager::isRecordConfirmed(const PerformanceRecord& record) {
    return (record.flags & UPLOAD_FLAG_CONFIRMED) != 0;
}

void NVRAMManager::markRecordUploaded(uint16_t address) {
    // Read current flags, set upload bit, write back only the flags byte
    uint8_t current_flags;
    EEPROM.get(address + offsetof(PerformanceRecord, flags), current_flags);
    current_flags |= UPLOAD_FLAG_SENT;
    EEPROM.put(address + offsetof(PerformanceRecord, flags), current_flags);
}

void NVRAMManager::markRecordConfirmed(uint16_t address) {
    // Read current flags, set confirmed bit, write back only the flags byte
    uint8_t current_flags;
    EEPROM.get(address + offsetof(PerformanceRecord, flags), current_flags);
    current_flags |= UPLOAD_FLAG_CONFIRMED;
    EEPROM.put(address + offsetof(PerformanceRecord, flags), current_flags);
}

void NVRAMManager::advanceWritePosition() {
    header.next_write_pos += RECORD_SIZE;
    
    // Wrap around if we've reached the end
    if (header.next_write_pos >= BUFFER_START + BUFFER_SIZE) {
        header.next_write_pos = BUFFER_START;
    }
    
    // If we've wrapped around and caught up to oldest record,
    // advance the oldest record pointer (overwriting old data)
    if (header.next_write_pos == header.oldest_record_pos) {
        header.oldest_record_pos += RECORD_SIZE;
        if (header.oldest_record_pos >= BUFFER_START + BUFFER_SIZE) {
            header.oldest_record_pos = BUFFER_START;
        }
    }
}

bool NVRAMManager::hasUnuploadedData() {
    uint16_t pos = header.oldest_record_pos;
    
    while (pos != header.next_write_pos) {
        PerformanceRecord record;
        EEPROM.get(pos, record);
        
        if (validateRecord(record) && !isRecordConfirmed(record)) {
            return true;
        }
        
        pos += RECORD_SIZE;
        if (pos >= BUFFER_START + BUFFER_SIZE) {
            pos = BUFFER_START;
        }
    }
    
    return false;
}

uint16_t NVRAMManager::getUnuploadedRecordCount() {
    uint16_t count = 0;
    uint16_t pos = header.oldest_record_pos;
    
    while (pos != header.next_write_pos) {
        PerformanceRecord record;
        EEPROM.get(pos, record);
        
        if (validateRecord(record) && !isRecordConfirmed(record)) {
            count++;
        }
        
        pos += RECORD_SIZE;
        if (pos >= BUFFER_START + BUFFER_SIZE) {
            pos = BUFFER_START;
        }
    }
    
    return count;
}

bool NVRAMManager::uploadPendingData() {
    if (!uploader) {
        return false;
    }
    
    last_upload_attempt = millis();
    bool any_uploaded = false;
    uint16_t pos = header.oldest_record_pos;
    uint8_t batch_count = 0;
    const uint8_t MAX_BATCH_SIZE = 10;  // Upload in batches to avoid overwhelming ESP32
    
    while (pos != header.next_write_pos && batch_count < MAX_BATCH_SIZE) {
        PerformanceRecord record;
        EEPROM.get(pos, record);
        
        if (validateRecord(record) && !isRecordUploaded(record)) {
            if (uploader->sendPerformanceData(record)) {
                // Mark as uploaded (minimal EEPROM wear - only 1 byte)
                markRecordUploaded(pos);
                any_uploaded = true;
                batch_count++;
            } else {
                // Upload failed, stop trying for now
                break;
            }
        }
        
        pos += RECORD_SIZE;
        if (pos >= BUFFER_START + BUFFER_SIZE) {
            pos = BUFFER_START;
        }
    }
    
    return any_uploaded;
}

void NVRAMManager::confirmUploaded(uint32_t timestamp) {
    uint16_t pos = header.oldest_record_pos;
    
    while (pos != header.next_write_pos) {
        PerformanceRecord record;
        EEPROM.get(pos, record);
        
        if (validateRecord(record) && record.timestamp == timestamp) {
            record.flags |= UPLOAD_FLAG_CONFIRMED;
            EEPROM.put(pos, record);
            break;
        }
        
        pos += RECORD_SIZE;
        if (pos >= BUFFER_START + BUFFER_SIZE) {
            pos = BUFFER_START;
        }
    }
    
    // Clean up confirmed records periodically
    cleanupUploadedRecords();
}

void NVRAMManager::cleanupUploadedRecords() {
    // Advance oldest_record_pos past confirmed uploads
    while (header.oldest_record_pos != header.next_write_pos) {
        PerformanceRecord record;
        EEPROM.get(header.oldest_record_pos, record);
        
        if (!validateRecord(record) || !(record.flags & UPLOAD_FLAG_CONFIRMED)) {
            break;  // Found unconfirmed record, stop cleanup
        }
        
        // This record is confirmed, we can "delete" it by advancing past it
        header.oldest_record_pos += RECORD_SIZE;
        if (header.oldest_record_pos >= BUFFER_START + BUFFER_SIZE) {
            header.oldest_record_pos = BUFFER_START;
        }
    }
    
    saveHeader();
}

void NVRAMManager::performMaintenance() {
    cleanupUploadedRecords();
    
    // Attempt upload if we have pending data and enough time has passed
    if (hasUnuploadedData() && 
        (millis() - last_upload_attempt > upload_retry_interval)) {
        uploadPendingData();
    }
}

uint16_t NVRAMManager::getAvailableSpace() {
    uint16_t used_records = 0;
    uint16_t pos = header.oldest_record_pos;
    
    while (pos != header.next_write_pos) {
        used_records++;
        pos += RECORD_SIZE;
        if (pos >= BUFFER_START + BUFFER_SIZE) {
            pos = BUFFER_START;
        }
    }
    
    return MAX_RECORDS - used_records;
}

void NVRAMManager::getStorageStats(uint16_t& used, uint16_t& available, uint16_t& pending_upload) {
    used = 0;
    pending_upload = 0;
    uint16_t pos = header.oldest_record_pos;
    
    while (pos != header.next_write_pos) {
        PerformanceRecord record;
        EEPROM.get(pos, record);
        
        if (validateRecord(record)) {
            used++;
            if (!(record.flags & UPLOAD_FLAG_CONFIRMED)) {
                pending_upload++;
            }
        }
        
        pos += RECORD_SIZE;
        if (pos >= BUFFER_START + BUFFER_SIZE) {
            pos = BUFFER_START;
        }
    }
    
    available = MAX_RECORDS - used;
}

float NVRAMManager::getEEPROMWearLevel() {
    // Estimate wear based on write cycles
    // EEPROM is rated for ~100,000 cycles
    return (float)header.write_cycles / 100000.0f;
}

void NVRAMManager::printStorageStatus() {
    uint16_t used, available, pending;
    getStorageStats(used, available, pending);
    
    Serial.println("=== NVRAM Storage Status ===");
    Serial.print("Total records stored: "); Serial.println(header.total_records);
    Serial.print("Records in buffer: "); Serial.println(used);
    Serial.print("Available space: "); Serial.println(available);
    Serial.print("Pending upload: "); Serial.println(pending);
    Serial.print("EEPROM wear level: "); Serial.print(getEEPROMWearLevel() * 100); Serial.println("%");
    Serial.print("Buffer utilization: "); Serial.print((float)used / MAX_RECORDS * 100); Serial.println("%");
}