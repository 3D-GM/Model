#pragma once

#include <cstdint>
#include "ChunkTypes.h"

/**
 * 3GM Chunk Header Structure
 * All data in little-endian format as validated in RFC
 */
struct ChunkHeader {
    uint32_t rawID;      // 4-byte chunk identifier (little-endian ASCII)
    uint32_t size;       // Data size in bytes (little-endian)
    ChunkType type;      // Parsed chunk type enum
    
    ChunkHeader() : rawID(0), size(0), type(ChunkType::Unknown) {}
    
    ChunkHeader(uint32_t id, uint32_t dataSize) 
        : rawID(id), size(dataSize), type(GetChunkTypeFromRawID(id)) {}
    
    /**
     * Check if this is a valid, non-empty chunk
     */
    bool IsValid() const {
        return rawID != 0 && type != ChunkType::Unknown;
    }
    
    /**
     * Check if this is the end-of-file marker
     */
    bool IsEndMarker() const {
        return type == ChunkType::End;
    }
    
    /**
     * Get total chunk size including header (8 bytes + data)
     */
    uint32_t GetTotalSize() const {
        return 8 + size;  // 8 bytes header + data size
    }
    
    /**
     * Get human-readable string for debugging
     */
    const char* GetName() const {
        return ChunkTypeToString(type);
    }
};