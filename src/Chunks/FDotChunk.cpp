#include "FDotChunk.h"
#include "ChunkHeader.h"
#include "ShapeData.h"
#include "VertexProcessor.h"
#include "ErrorHandler.h"
#include <cstring>

/**
 * FDot Chunk Processor
 * Handles compressed vertex data using RFC-validated DecrunchDots algorithm
 * Expansion ratio: 6 bytes → 32 bytes per vertex (5.33x expansion)
 */

bool FDotChunkProcessor::ProcessChunk(const ChunkHeader& header, 
                                     const uint8_t* data, 
                                     ShapeData& shape) {
    if (!ValidateChunkData(header, data)) {
        return ErrorHandler::PostEvent(0x6A, "Invalid FDot chunk data");
    }
    
    // RFC VALIDATED: Calculate vertex count
    // FDot format: 24 bytes compression params + 6 bytes per vertex
    size_t vertexCount = CalculateVertexCount(header.size);
    if (vertexCount == 0) {
        return ErrorHandler::PostEvent(0x6A, "No vertices in FDot chunk");
    }
    
    // Allocate vertex buffer (8 floats per vertex as per RFC validation)
    shape.AllocateVertexBuffer(vertexCount);
    float* outputVertices = shape.GetVertexBuffer();
    
    // RFC VALIDATED: Use DecrunchDots algorithm
    if (!VertexProcessor::DecrunchDotsVertices(data, outputVertices, vertexCount)) {
        return ErrorHandler::PostEvent(0x6A, "Failed to decompress FDot vertices");
    }
    
    shape.SetVertexCount(vertexCount);
    return true;
}

size_t FDotChunkProcessor::CalculateVertexCount(size_t chunkSize) const {
    // RFC VALIDATED: DecrunchDots format
    // 24 bytes = 6 compression parameters (4 bytes each)
    // Remaining data = compressed vertex data (6 bytes per vertex: 3 × int16)
    if (chunkSize < 24) {
        return 0;  // Invalid - not enough data for compression parameters
    }
    
    size_t vertexDataSize = chunkSize - 24;
    if (vertexDataSize % 6 != 0) {
        return 0;  // Invalid - vertex data not multiple of 6 bytes
    }
    
    return vertexDataSize / 6;
}

bool FDotChunkProcessor::ValidateChunkData(const ChunkHeader& header, 
                                          const uint8_t* data) const {
    if (!data) {
        return false;
    }
    
    if (header.type != ChunkType::FDot) {
        return false;
    }
    
    if (header.size < 24) {  // Minimum: 24 bytes compression params
        return false;
    }
    
    // Vertex data must be multiple of 6 bytes (3 × int16 per vertex)
    if ((header.size - 24) % 6 != 0) {
        return false;
    }
    
    return true;
}