#pragma once

#include "ChunkProcessor.h"
#include <cstdint>

/**
 * FDot Chunk Processor
 * Handles compressed vertex data using RFC-validated DecrunchDots algorithm
 */
class FDotChunkProcessor : public ChunkProcessor {
public:
    bool ProcessChunk(const ChunkHeader& header, 
                     const uint8_t* data, 
                     ShapeData& shape) override;
    
    ChunkType GetChunkType() const override { 
        return ChunkType::FDot; 
    }
    
    const char* GetChunkName() const override { 
        return "FDot"; 
    }
    
    bool ValidateChunkData(const ChunkHeader& header, 
                          const uint8_t* data) const override;

private:
    /**
     * Calculate vertex count from FDot chunk size
     * RFC validated: (size - 24) / 6 bytes per vertex
     */
    size_t CalculateVertexCount(size_t chunkSize) const;
};