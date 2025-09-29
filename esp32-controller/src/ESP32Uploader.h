#pragma once

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

/**
 * ESP32 Arduino Nano Programming Interface
 * 
 * Handles over-the-air programming of Arduino Nano via UART
 * - Stores Nano firmware in ESP32 SPIFFS
 * - Implements STK500 protocol for programming
 * - Provides progress feedback via WiFi/BLE
 */
class ESP32Uploader {
public:
    enum class UploadState {
        IDLE,
        CONNECTING,
        UPLOADING,
        VERIFYING,
        COMPLETE,
        ERROR
    };

    struct UploadProgress {
        UploadState state;
        size_t bytesTotal;
        size_t bytesWritten;
        float percentComplete;
        String errorMessage;
    };

    ESP32Uploader(HardwareSerial& nanoSerial, uint8_t resetPin, uint8_t dtrctsPin);
    
    // Firmware management
    bool storeFirmware(const uint8_t* hexData, size_t hexSize, const String& version);
    bool loadStoredFirmware(const String& version);
    bool uploadToNano(const String& firmwareVersion = "");
    
    // Progress monitoring
    UploadProgress getProgress() const { return progress_; }
    void setProgressCallback(std::function<void(const UploadProgress&)> callback);
    
    // STK500 Protocol implementation
    bool enterProgrammingMode();
    bool exitProgrammingMode();
    bool flashPage(uint16_t address, const uint8_t* data, size_t length);
    bool verifyFlash();
    
private:
    HardwareSerial& nanoSerial_;
    uint8_t resetPin_;
    uint8_t dtrctsPin_;
    UploadProgress progress_;
    std::function<void(const UploadProgress&)> progressCallback_;
    
    // STK500 helpers
    bool sendSTKCommand(uint8_t cmd, const uint8_t* data = nullptr, size_t dataLen = 0);
    bool waitForSTKResponse(uint8_t expectedResponse, uint32_t timeoutMs = 1000);
    void resetNano();
    void enterBootloader();
};