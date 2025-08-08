#pragma once

#include "ChunkProcessor.h"
#include <cstdint>

/**
 * Prim Chunk Processor
 * Handles simple primitive data using RFC-validated algorithms
 * Different from Line chunks - uses direct primitive processing
 */
class PrimChunkProcessor : public ChunkProcessor {
public:
    bool ProcessChunk(const ChunkHeader& header, 
                     const uint8_t* data, 
                     ShapeData& shape) override;
    
    ChunkType GetChunkType() const override { 
        return ChunkType::Prim; 
    }
    
    const char* GetChunkName() const override { 
        return "Prim"; 
    }
    
    bool ValidateChunkData(const ChunkHeader& header, 
                          const uint8_t* data) const override;

private:
    /**
     * Parse primitive data from chunk
     * @param data Raw chunk data
     * @param size Chunk size
     * @param shape Target shape to populate
     * @return true if parsing succeeded
     */
    bool ParsePrimitiveData(const uint8_t* data, size_t size, ShapeData& shape);
};