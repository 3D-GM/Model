#include "HeaderDetector.h"
#include "ByteSwap.h"
#include "ErrorHandler.h"
#include <iostream>

FileHeader HeaderDetector::DetectHeader(const uint8_t* data, size_t fileSize) {
    FileHeader header;
    
    if (!data || fileSize < 4) {
        ErrorHandler::PostEvent(0x6A, "Invalid file data for header detection");
        return header;
    }
    
    // Read first 4 bytes as little-endian
    uint32_t first4bytes = ByteSwap::ReadLittleEndian32(data);
    
    // RFC VALIDATED: Header detection algorithm
    header.type = DetectHeaderType(first4bytes);
    
    switch (header.type) {
        case HeaderType::FullHeader: {
            // Full header: "3DGM" + version + info (12 bytes total)
            if (fileSize < 12) {
                ErrorHandler::PostEvent(0x6A, "File too small for full header");
                header.type = HeaderType::NoHeader;
                return header;
            }
            
            header.magic = first4bytes;  // 0x4D474433 = "3DGM"
            header.version = ByteSwap::ReadLittleEndian32(data + 4);
            header.info = ByteSwap::ReadLittleEndian32(data + 8);
            header.headerSize = 12;
            header.chunkOffset = 12;
            break;
        }
        
        case HeaderType::VersionOnly: {
            // Version-only header: 4 bytes version
            header.magic = 0;
            header.version = first4bytes;
            header.info = 0;
            header.headerSize = 4;
            header.chunkOffset = 4;
            break;
        }
        
        case HeaderType::NoHeader: {
            // No header, chunks start immediately
            header.magic = 0;
            header.version = 0;
            header.info = 0;
            header.headerSize = 0;
            header.chunkOffset = 0;
            break;
        }
    }
    
    return header;
}

HeaderType HeaderDetector::DetectHeaderType(uint32_t first4bytes) {
    // RFC VALIDATED: Check for "3DGM" magic first
    if (first4bytes == 0x4D474433) {  // "3DGM" in little-endian
        return HeaderType::FullHeader;
    }
    
    // RFC VALIDATED: Check for version-only header range
    if (IsValidVersionRange(first4bytes)) {
        return HeaderType::VersionOnly;
    }
    
    // No recognizable header pattern
    return HeaderType::NoHeader;
}

bool HeaderDetector::IsValidVersionRange(uint32_t value) {
    // RFC VALIDATED: Version range 0x01000100 to 0x10000100
    // Examples from validation:
    // - 0x03000100 = Version 65539 (ball_missile.3GM)
    // - 0x04000100 = Version 65540 (ammo_box.3GM)
    return (value >= 0x01000100 && value <= 0x10000100);
}

bool HeaderDetector::ValidateHeader(const FileHeader& header, const uint8_t* data, size_t fileSize) {
    if (!data) {
        return false;
    }
    
    // Check that chunk offset doesn't exceed file size
    if (header.chunkOffset >= fileSize) {
        return false;
    }
    
    // For full headers, validate magic number
    if (header.type == HeaderType::FullHeader) {
        if (header.magic != 0x4D474433) {  // "3DGM"
            return false;
        }
    }
    
    // For version headers, validate version range
    if (header.type == HeaderType::VersionOnly) {
        if (!IsValidVersionRange(header.version)) {
            return false;
        }
    }
    
    // Validate that remaining data can contain at least one chunk header
    size_t remainingData = fileSize - header.chunkOffset;
    if (remainingData < 8) {  // Minimum chunk header size
        return false;
    }
    
    return true;
}