#include "Dot2Chunk.h"
#include "ChunkHeader.h" 
#include "ShapeData.h"
#include "VertexProcessor.h"
#include "ErrorHandler.h"
#include <cstring>

/**
 * Dot2 Chunk Processor
 * Handles original format vertex coordinate data
 * Based on RFC validation of convertPackedToFloatVertices algorithms
 */

bool Dot2ChunkProcessor::ProcessChunk(const ChunkHeader& header, 
                                    const uint8_t* data, 
                                    ShapeData& shape) {
    if (!ValidateChunkData(header, data)) {
        return ErrorHandler::PostEvent(0x6A, "Invalid Dot2 chunk data");
    }
    
    // Calculate vertex count: (size - 8) / 12 bytes per vertex
    // 8 bytes = compression parameters (skipped)
    // 12 bytes = 3 * uint32_t packed coordinates per vertex
    if (header.size < 8) {
        return ErrorHandler::PostEvent(0x6A, "Dot2 chunk too small");
    }
    
    size_t vertexDataSize = header.size - 8;
    if (vertexDataSize % 12 != 0) {
        return ErrorHandler::PostEvent(0x6A, "Invalid Dot2 vertex data size");
    }
    
    size_t vertexCount = vertexDataSize / 12;
    
    // Skip compression parameters (8 bytes) as validated in RFC
    const uint32_t* packedVertices = reinterpret_cast<const uint32_t*>(data + 8);
    
    // Allocate vertex buffer (8 floats per vertex as per RFC validation)
    shape.AllocateVertexBuffer(vertexCount);
    float* outputVertices = shape.GetVertexBuffer();
    
    // RFC VALIDATED: Use convertPackedToFloatVertices algorithm
    if (!VertexProcessor::ConvertPackedToFloatVertices(packedVertices, outputVertices, vertexCount)) {
        return ErrorHandler::PostEvent(0x6A, "Failed to process Dot2 vertices");
    }
    
    shape.SetVertexCount(vertexCount);
    return true;
}

bool Dot2ChunkProcessor::ProcessVertexData(const uint32_t* packedVertices, 
                                          float* outputVertices, 
                                          size_t vertexCount) {
    float* v4 = outputVertices;
    
    for (size_t i = 0; i < vertexCount; i++) {
        // RFC VALIDATED: convertPackedToFloatVertices algorithm
        // Complex pointer arithmetic with backward references
        
        // Process X coordinate from current position  
        uint32_t xPacked = ByteSwap::ApplyComplexByteSwap(*packedVertices);
        
        // CRITICAL: Jump input pointer forward by 3 DWORDs
        packedVertices += 3;
        
        // Store X as float at current output position
        *v4 = static_cast<float>(xPacked);
        
        // CRITICAL: Jump output pointer forward by 8 floats
        v4 += 8;
        
        // Process Y coordinate from 2 positions BACK in input
        uint32_t yPacked = ByteSwap::ApplyComplexByteSwap(*(packedVertices - 2));
        
        // Store Y at output position 7 floats BACK
        *(v4 - 7) = static_cast<float>(yPacked);
        
        // Process Z coordinate from 1 position BACK in input
        uint32_t zPacked = ByteSwap::ApplyComplexByteSwap(*(packedVertices - 1));
        
        // Store Z at output position 6 floats BACK  
        *(v4 - 6) = static_cast<float>(zPacked);
    }
    
    // Add terminator as validated in RFC
    *v4 = GlobalVariables::GetVertexTerminator();  // dword_96BD28
    
    return true;
}

bool Dot2ChunkProcessor::ValidateChunkData(const ChunkHeader& header, 
                                          const uint8_t* data) const {
    if (!data) {
        return false;
    }
    
    if (header.type != ChunkType::Dot2) {
        return false;
    }
    
    if (header.size < 8) {  // Minimum: 8 bytes compression params
        return false;
    }
    
    // Vertex data must be multiple of 12 bytes (3 packed coordinates)
    if ((header.size - 8) % 12 != 0) {
        return false;
    }
    
    return true;
}