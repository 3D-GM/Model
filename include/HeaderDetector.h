#pragma once

#include <cstdint>

/**
 * 3GM Header Detection System
 * Based on RFC validation of dual header formats
 */

enum class HeaderType {
    NoHeader,       // No header, chunks start immediately
    VersionOnly,    // 4-byte version header  
    FullHeader      // 12-byte full header with magic + version + info
};

struct FileHeader {
    HeaderType type;
    uint32_t magic;         // "3DGM" magic (0x4D474433) for full headers
    uint32_t version;       // Version number (little-endian)
    uint32_t info;          // Info field (only in full headers)
    size_t headerSize;      // Total header size in bytes
    size_t chunkOffset;     // Offset where chunks begin
    
    FileHeader() : type(HeaderType::NoHeader), magic(0), version(0), info(0), headerSize(0), chunkOffset(0) {}
    
    bool IsValid() const {
        return type != HeaderType::NoHeader;
    }
    
    bool HasMagic() const {
        return type == HeaderType::FullHeader && magic == 0x4D474433;
    }
};

/**
 * Header detector class implementing RFC-validated detection algorithm
 */
class HeaderDetector {
public:
    /**
     * Detect header type and parse header data
     * @param data File data buffer (at least 12 bytes)
     * @param fileSize Total file size for validation
     * @return Parsed header information
     */
    static FileHeader DetectHeader(const uint8_t* data, size_t fileSize);
    
    /**
     * Validate detected header against file content
     * @param header Detected header
     * @param data File data
     * @param fileSize File size
     * @return true if header is consistent with file structure
     */
    static bool ValidateHeader(const FileHeader& header, const uint8_t* data, size_t fileSize);
    
private:
    /**
     * RFC VALIDATED: Header detection algorithm
     * Tests first 4 bytes for magic or version range
     */
    static HeaderType DetectHeaderType(uint32_t first4bytes);
    
    /**
     * Check if value is in valid version range
     * RFC validated range: 0x01000100 to 0x10000100
     */
    static bool IsValidVersionRange(uint32_t value);
};