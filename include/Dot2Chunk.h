#pragma once

#include "ChunkProcessor.h"
#include <cstdint>

/**
 * Dot2 Chunk Processor
 * Handles original format vertex coordinate data using validated 
 * convertPackedToFloatVertices algorithm from RFC
 */
class Dot2ChunkProcessor : public ChunkProcessor {
public:
    bool ProcessChunk(const ChunkHeader& header, 
                     const uint8_t* data, 
                     ShapeData& shape) override;
    
    ChunkType GetChunkType() const override { 
        return ChunkType::Dot2; 
    }
    
    const char* GetChunkName() const override { 
        return "Dot2"; 
    }
    
    bool ValidateChunkData(const ChunkHeader& header, 
                          const uint8_t* data) const override;

private:
    /**
     * Process packed vertex data using RFC-validated algorithm
     * Implements complex pointer arithmetic with backward references
     * @param packedVertices Input packed vertex data (3 uint32 per vertex)
     * @param outputVertices Output float buffer (8 floats per vertex) 
     * @param vertexCount Number of vertices to process
     * @return true if successful
     */
    bool ProcessVertexData(const uint32_t* packedVertices,
                          float* outputVertices, 
                          size_t vertexCount);
};