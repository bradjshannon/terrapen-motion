# Arduino Nano Upload via ESP32 Bridge - PowerShell Version
# Uploads Arduino firmware through ESP32 controller using HTTP interface

param(
    [Parameter(Mandatory=$true)]
    [string]$HexFile
)

# ESP32 controller settings
$PossibleIPs = @("192.168.4.1", "192.168.1.1", "10.0.0.1")
$ESP32_IP = $null
$UploadURL = $null
$StatusURL = $null

function Wait-ForESP32 {
    Write-Host "Waiting for ESP32 controller..." -ForegroundColor Yellow
    
    # Try different common ESP32 AP IP addresses
    foreach ($ip in $PossibleIPs) {
        Write-Host "Trying IP: $ip" -ForegroundColor Cyan
        try {
            $testUrl = "http://$ip/status"
            $response = Invoke-RestMethod -Uri $testUrl -TimeoutSec 3 -ErrorAction Stop
            
            $script:ESP32_IP = $ip
            $script:UploadURL = "http://$ip/upload-hex"
            $script:StatusURL = "http://$ip/status"
            Write-Host "✅ ESP32 controller found at $ip!" -ForegroundColor Green
            return $true
        }
        catch {
            # Continue to next IP
        }
    }
    
    # If not found, wait a bit for ESP32 to boot
    Write-Host "ESP32 not found immediately, waiting for boot..." -ForegroundColor Yellow
    for ($i = 1; $i -le 20; $i++) {
        foreach ($ip in $PossibleIPs) {
            try {
                $testUrl = "http://$ip/status"
                $response = Invoke-RestMethod -Uri $testUrl -TimeoutSec 2 -ErrorAction Stop
                
                $script:ESP32_IP = $ip
                $script:UploadURL = "http://$ip/upload-hex"
                $script:StatusURL = "http://$ip/status"
                Write-Host "✅ ESP32 controller found at $ip!" -ForegroundColor Green
                return $true
            }
            catch {
                # Continue
            }
        }
        Start-Sleep -Seconds 1
        Write-Host "  Attempt $i/20..." -ForegroundColor Gray
    }
    
    Write-Host "❌ ERROR: ESP32 controller not found!" -ForegroundColor Red
    Write-Host "Make sure:" -ForegroundColor Yellow
    Write-Host "1. ESP32 is powered on and running"
    Write-Host "2. You're connected to 'TerraPen-Robot' WiFi network"
    Write-Host "3. ESP32 firmware is running (should see blinking LED)"
    return $false
}

function Upload-Firmware {
    param([string]$hexFilePath)
    
    if (-not (Test-Path $hexFilePath)) {
        Write-Host "❌ ERROR: Firmware file not found: $hexFilePath" -ForegroundColor Red
        return $false
    }
    
    $fileSize = (Get-Item $hexFilePath).Length
    $fileName = Split-Path $hexFilePath -Leaf
    
    Write-Host "📁 Uploading firmware: $fileName" -ForegroundColor Cyan
    Write-Host "📊 File size: $fileSize bytes" -ForegroundColor Cyan
    
    try {
        # Read file content
        $fileContent = [System.IO.File]::ReadAllBytes($hexFilePath)
        
        # Create multipart form data
        $boundary = [System.Guid]::NewGuid().ToString()
        $LF = "`r`n"
        
        $bodyLines = (
            "--$boundary",
            "Content-Disposition: form-data; name=`"firmware`"; filename=`"$fileName`"",
            "Content-Type: application/octet-stream$LF",
            [System.Text.Encoding]::GetEncoding("iso-8859-1").GetString($fileContent),
            "--$boundary--$LF"
        ) -join $LF
        
        Write-Host "🚀 Starting upload to Arduino via ESP32..." -ForegroundColor Green
        Write-Host "⏳ This may take 10-30 seconds depending on firmware size..." -ForegroundColor Yellow
        
        $response = Invoke-RestMethod -Uri $UploadURL -Method Post -Body $bodyLines -ContentType "multipart/form-data; boundary=$boundary" -TimeoutSec 90
        
        Write-Host "✅ Arduino firmware upload successful!" -ForegroundColor Green
        return $true
    }
    catch {
        Write-Host "❌ Upload failed: $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

# Main execution
Write-Host "=" * 50 -ForegroundColor Cyan
Write-Host "Arduino Nano Upload via ESP32 Bridge" -ForegroundColor Cyan
Write-Host "=" * 50 -ForegroundColor Cyan

# Convert .elf to .hex if needed
if ($HexFile.EndsWith('.elf')) {
    $HexFile = $HexFile.Replace('.elf', '.hex')
    if (-not (Test-Path $HexFile)) {
        Write-Host "Converting ELF to HEX..." -ForegroundColor Yellow
        $elfFile = $args[0]
        & avr-objcopy -O ihex "`"$elfFile`"" "`"$HexFile`""
    }
}

# Check ESP32 availability
if (-not (Wait-ForESP32)) {
    exit 1
}

# Upload firmware
if (Upload-Firmware $HexFile) {
    Write-Host "🎉 Arduino programming complete!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "💥 Arduino programming failed!" -ForegroundColor Red
    exit 1
}