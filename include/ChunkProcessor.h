#pragma once

#include <cstdint>
#include <memory>
#include "ChunkTypes.h"

// Forward declarations
struct ChunkHeader;
class ShapeData;

/**
 * Base interface for all chunk processors
 * Each chunk type (Dot2, Line, soPF, etc.) implements this interface
 */
class ChunkProcessor {
public:
    virtual ~ChunkProcessor() = default;
    
    /**
     * Process a chunk of data
     * @param header Parsed chunk header with ID and size
     * @param data Raw chunk data (header.size bytes)
     * @param shape Target shape to populate with data
     * @return true if processing succeeded, false on error
     */
    virtual bool ProcessChunk(const ChunkHeader& header, 
                            const uint8_t* data, 
                            ShapeData& shape) = 0;
    
    /**
     * Get the chunk type this processor handles
     * @return ChunkType enum value
     */
    virtual ChunkType GetChunkType() const = 0;
    
    /**
     * Get human-readable name for debugging
     * @return Chunk type name (e.g. "Dot2", "Line", "soPF")
     */
    virtual const char* GetChunkName() const = 0;
    
    /**
     * Validate chunk data before processing
     * @param header Chunk header
     * @param data Raw chunk data
     * @return true if data is valid for this chunk type
     */
    virtual bool ValidateChunkData(const ChunkHeader& header, 
                                 const uint8_t* data) const = 0;
};