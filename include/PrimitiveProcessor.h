#pragma once

#include "PrimitiveTypes.h"
#include <cstdint>
#include <cstddef>
#include <vector>

// Forward declarations
class ShapeData;

/**
 * Primitive processing system based on RFC validation
 * Handles all 7 primitive types with correct flag patterns
 */
class PrimitiveProcessor {
public:
    /**
     * Process primitive data using RFC-validated algorithms
     * @param primitiveData Input primitive data
     * @param dataSize Size of primitive data
     * @param shape Target shape to populate
     * @return true if processing succeeded
     */
    static bool ProcessPrimitiveData(const uint16_t* primitiveData, 
                                   size_t dataSize,
                                   ShapeData& shape);
    
    /**
     * RFC VALIDATED: Extract primitive data (extractPrimitiveData function)
     * @param inputData Input primitive buffer
     * @param outputBuffer Output buffer for processed data
     * @param extractCount Number of primitives to extract
     * @return true if extraction succeeded
     */
    static bool ExtractPrimitiveData(const uint32_t* inputData,
                                   uint32_t* outputBuffer, 
                                   int extractCount);
    
    /**
     * RFC VALIDATED: Create surface from primitive (createSurfaceFromPrimitive function)
     * @param primitiveData Primitive data buffer
     * @param surfaceBuffer Surface creation buffer
     * @return true if surface creation succeeded
     */
    static bool CreateSurfaceFromPrimitive(const uint32_t* primitiveData,
                                         const uint32_t* surfaceBuffer);
    
    /**
     * Set primitive type flags in global register
     * RFC VALIDATED: Updates dword_9668EC with correct flag patterns
     */
    static void SetPrimitiveFlags(PrimitiveType type);
    
    /**
     * Parse primitive type from data stream
     * @param data Input data stream
     * @param offset Current offset in stream
     * @param parsedType Output primitive type
     * @return true if parsing succeeded
     */
    static bool ParsePrimitiveType(const uint16_t* data, 
                                 size_t offset,
                                 PrimitiveType& parsedType);
    
    /**
     * Validate primitive data structure
     * @param primitiveData Data to validate
     * @param dataSize Size of data
     * @return true if data is valid
     */
    static bool ValidatePrimitiveData(const uint16_t* primitiveData, size_t dataSize);
    
    /**
     * Count primitives in data stream
     * RFC VALIDATED: Based on gm_CountPrimitives.cpp
     * @param primitiveData Input data
     * @param dataSize Data size
     * @return Number of primitives found
     */
    static size_t CountPrimitives(const uint16_t* primitiveData, size_t dataSize);
    
private:
    /**
     * Process specific primitive type with appropriate algorithm
     */
    static bool ProcessTriangleStrip(const uint16_t* data, size_t count, ShapeData& shape);
    static bool ProcessTriangleList(const uint16_t* data, size_t count, ShapeData& shape);
    static bool ProcessQuadStrip(const uint16_t* data, size_t count, ShapeData& shape);
    static bool ProcessPointSprite(const uint16_t* data, size_t count, ShapeData& shape);
    static bool ProcessLineStrip(const uint16_t* data, size_t count, ShapeData& shape);
    static bool ProcessComplexPrimitive(const uint16_t* data, size_t count, ShapeData& shape);
    
    /**
     * Convert primitive indices for different rendering types
     */
    static std::vector<uint16_t> ConvertToTriangleList(const uint16_t* stripData, size_t count);
    static std::vector<uint16_t> ConvertQuadToTriangles(const uint16_t* quadData, size_t count);
};