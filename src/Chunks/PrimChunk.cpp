#include "PrimChunk.h"
#include "ChunkHeader.h"
#include "ShapeData.h"
#include "PrimitiveProcessor.h"
#include "ErrorHandler.h"
#include <cstring>

/**
 * Prim Chunk Processor
 * Handles simple primitive chunks (vs Line chunks which are complex)
 * Uses direct primitive processing without 4-phase pipeline
 */

bool PrimChunkProcessor::ProcessChunk(const ChunkHeader& header, 
                                     const uint8_t* data, 
                                     ShapeData& shape) {
    if (!ValidateChunkData(header, data)) {
        return ErrorHandler::PostEvent(0x6A, "Invalid Prim chunk data");
    }
    
    // RFC DIFFERENCE: Prim chunks use direct processing
    // Line chunks use 4-phase convertChunkedDataToSurfaces pipeline
    if (!ParsePrimitiveData(data, header.size, shape)) {
        return ErrorHandler::PostEvent(0x6A, "Failed to parse Prim chunk data");
    }
    
    // Set shape processing flag (NOT the Line-processed flag bit 3)
    // Prim chunks use different processing path
    uint32_t currentFlags = shape.GetShapeFlags();
    shape.SetShapeFlags(currentFlags | 0x04);  // Set bit 2 for Prim-processed
    
    return true;
}

bool PrimChunkProcessor::ParsePrimitiveData(const uint8_t* data, size_t size, ShapeData& shape) {
    if (!data || size == 0) {
        return false;
    }
    
    // Convert byte data to uint16 primitive data
    const uint16_t* primitiveData = reinterpret_cast<const uint16_t*>(data);
    size_t primitiveCount = size / sizeof(uint16_t);
    
    // RFC VALIDATED: Use PrimitiveProcessor for all primitive handling
    return PrimitiveProcessor::ProcessPrimitiveData(primitiveData, primitiveCount, shape);
}

bool PrimChunkProcessor::ValidateChunkData(const ChunkHeader& header, 
                                          const uint8_t* data) const {
    if (!data) {
        return false;
    }
    
    if (header.type != ChunkType::Prim) {
        return false;
    }
    
    if (header.size == 0) {
        return false;  // Empty primitive data is invalid
    }
    
    // Primitive data should be even number of bytes (uint16 aligned)
    if (header.size % 2 != 0) {
        return false;
    }
    
    return true;
}