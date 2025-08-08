#pragma once

#include "ChunkHeader.h"
#include <vector>
#include <cstdint>

/**
 * Chunk reading and traversal system
 * Implements RFC-validated chunk structure parsing
 */
class ChunkReader {
private:
    const uint8_t* fileData_;
    size_t fileSize_;
    size_t currentOffset_;
    std::vector<ChunkHeader> discoveredChunks_;
    
public:
    ChunkReader(const uint8_t* data, size_t size, size_t startOffset = 0);
    
    /**
     * Scan entire file and discover all chunks
     * @return true if scanning succeeded
     */
    bool ScanAllChunks();
    
    /**
     * Read next chunk header from current position
     * @param header Output chunk header
     * @return true if chunk header was read successfully
     */
    bool ReadNextChunkHeader(ChunkHeader& header);
    
    /**
     * Get chunk data pointer for given header
     * @param header Chunk header from ReadNextChunkHeader
     * @return Pointer to chunk data (after 8-byte header)
     */
    const uint8_t* GetChunkData(const ChunkHeader& header) const;
    
    /**
     * Skip to next chunk after current one
     * @param header Current chunk header
     * @return true if skip succeeded
     */
    bool SkipToNextChunk(const ChunkHeader& header);
    
    /**
     * Reset reader to beginning of chunk area
     */
    void Reset();
    
    /**
     * Get all discovered chunks
     */
    const std::vector<ChunkHeader>& GetDiscoveredChunks() const { return discoveredChunks_; }
    
    /**
     * Get current read offset
     */
    size_t GetCurrentOffset() const { return currentOffset_; }
    
    /**
     * Check if we've reached end of chunks
     * @return true if at end or past end
     */
    bool IsAtEnd() const;
    
    /**
     * Validate chunk structure integrity
     * @return true if all chunks have valid structure
     */
    bool ValidateChunkStructure() const;
    
    /**
     * Print debug information about all discovered chunks
     */
    void PrintChunkSummary() const;
};