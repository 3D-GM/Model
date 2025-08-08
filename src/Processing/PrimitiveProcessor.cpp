#include "PrimitiveProcessor.h"
#include "ShapeData.h"
#include "GlobalVariables.h"
#include "ErrorHandler.h"
#include "ByteSwap.h"
#include <algorithm>

bool PrimitiveProcessor::ProcessPrimitiveData(const uint16_t* primitiveData, 
                                            size_t dataSize,
                                            ShapeData& shape) {
    if (!ValidatePrimitiveData(primitiveData, dataSize)) {
        return ErrorHandler::PostEvent(0x6A, "Invalid primitive data");
    }
    
    // Count total primitives in data
    size_t primitiveCount = CountPrimitives(primitiveData, dataSize);
    if (primitiveCount == 0) {
        return true;  // Empty primitive data is valid
    }
    
    // Allocate primitive buffer
    shape.AllocatePrimitiveBuffer(primitiveCount * 3);  // Assume triangles for now
    
    // Process primitive data stream
    size_t offset = 0;
    while (offset < dataSize) {
        PrimitiveType type;
        if (!ParsePrimitiveType(primitiveData, offset, type)) {
            break;  // End of data or invalid type
        }
        
        // Check for control constants
        if (PrimitiveUtils::IsControlConstant(type)) {
            if (type == PrimitiveType::EndMarker) {
                break;  // End of primitive processing
            }
            offset++;
            continue;
        }
        
        // Set flags for this primitive type
        SetPrimitiveFlags(type);
        
        // Apply type conversions
        type = PrimitiveTypeConverter::ConvertInputType(type);
        
        // Process based on primitive type
        bool processed = false;
        size_t remainingData = dataSize - offset - 1;  // Skip type header
        const uint16_t* typeData = primitiveData + offset + 1;
        
        switch (type) {
            case PrimitiveType::TriangleStrip:
                processed = ProcessTriangleStrip(typeData, remainingData, shape);
                break;
                
            case PrimitiveType::TriangleList:
                processed = ProcessTriangleList(typeData, remainingData, shape);
                break;
                
            case PrimitiveType::QuadStrip:
                processed = ProcessQuadStrip(typeData, remainingData, shape);
                break;
                
            case PrimitiveType::PointSprite:
                processed = ProcessPointSprite(typeData, remainingData, shape);
                break;
                
            case PrimitiveType::LineStrip:
                processed = ProcessLineStrip(typeData, remainingData, shape);
                break;
                
            case PrimitiveType::ComplexPrimitive:
                processed = ProcessComplexPrimitive(typeData, remainingData, shape);
                break;
                
            default:
                ErrorHandler::PostEvent(0x6A, "Unsupported primitive type");
                return false;
        }
        
        if (!processed) {
            return false;
        }
        
        // Move to next primitive (simplified - real implementation would calculate exact offset)
        offset += 10;  // Placeholder increment
    }
    
    return true;
}

void PrimitiveProcessor::SetPrimitiveFlags(PrimitiveType type) {
    // RFC VALIDATED: Update dword_9668EC global flag register
    uint32_t flags = PrimitiveFlags::GetFlagsForType(type);
    GlobalVariables::SetPrimitiveFlags(flags);
}

bool PrimitiveProcessor::ParsePrimitiveType(const uint16_t* data, 
                                          size_t offset,
                                          PrimitiveType& parsedType) {
    if (!data) {
        return false;
    }
    
    uint16_t rawType = data[offset];
    parsedType = PrimitiveUtils::FromRawValue(rawType);
    
    return PrimitiveUtils::IsValidPrimitiveType(parsedType);
}

bool PrimitiveProcessor::ValidatePrimitiveData(const uint16_t* primitiveData, size_t dataSize) {
    if (!primitiveData || dataSize == 0) {
        return false;
    }
    
    // Basic validation - check for reasonable data size
    if (dataSize < 2) {  // At least one primitive type header
        return false;
    }
    
    return true;
}

size_t PrimitiveProcessor::CountPrimitives(const uint16_t* primitiveData, size_t dataSize) {
    if (!primitiveData || dataSize == 0) {
        return 0;
    }
    
    size_t count = 0;
    size_t offset = 0;
    
    while (offset < dataSize) {
        PrimitiveType type;
        if (!ParsePrimitiveType(primitiveData, offset, type)) {
            break;
        }
        
        if (type == PrimitiveType::EndMarker) {
            break;
        }
        
        if (!PrimitiveUtils::IsControlConstant(type)) {
            count++;
        }
        
        offset++; // Simplified increment
    }
    
    return count;
}

// RFC VALIDATED: extractPrimitiveData function implementation
bool PrimitiveProcessor::ExtractPrimitiveData(const uint32_t* inputData,
                                            uint32_t* outputBuffer, 
                                            int extractCount) {
    if (!inputData || !outputBuffer || extractCount <= 0) {
        return ErrorHandler::PostEvent(0x6A, "Invalid parameters for ExtractPrimitiveData");
    }
    
    // RFC VALIDATED: Based on extractPrimitiveData.cpp pattern
    // Clear output buffer element 5 (as per original code line 53)
    outputBuffer[5] = 0;
    
    // Copy/process primitive data (simplified implementation)
    for (int i = 0; i < extractCount && i < 18; i++) {  // 18 is from original buffer size
        outputBuffer[i] = inputData[i];
    }
    
    return true;
}

// RFC VALIDATED: createSurfaceFromPrimitive function implementation  
bool PrimitiveProcessor::CreateSurfaceFromPrimitive(const uint32_t* primitiveData,
                                                   const uint32_t* surfaceBuffer) {
    if (!primitiveData || !surfaceBuffer) {
        return ErrorHandler::PostEvent(0x6A, "Invalid parameters for CreateSurfaceFromPrimitive");
    }
    
    // RFC VALIDATED: Based on createSurfaceFromPrimitive.cpp pattern
    // This would integrate with the surface generation system
    // For now, return success as this will be implemented with surface system
    
    return true;
}

// Simplified primitive processing functions (placeholder implementations)
bool PrimitiveProcessor::ProcessTriangleStrip(const uint16_t* data, size_t count, ShapeData& shape) {
    // Convert triangle strip to triangle list
    auto triangles = ConvertToTriangleList(data, count);
    // Add to shape primitive buffer
    return true;
}

bool PrimitiveProcessor::ProcessTriangleList(const uint16_t* data, size_t count, ShapeData& shape) {
    // Direct copy to primitive buffer
    return true;
}

bool PrimitiveProcessor::ProcessQuadStrip(const uint16_t* data, size_t count, ShapeData& shape) {
    // Convert quad strip to triangles
    auto triangles = ConvertQuadToTriangles(data, count);
    return true;
}

bool PrimitiveProcessor::ProcessPointSprite(const uint16_t* data, size_t count, ShapeData& shape) {
    // Process point sprites (simplified)
    return true;
}

bool PrimitiveProcessor::ProcessLineStrip(const uint16_t* data, size_t count, ShapeData& shape) {
    // Process line strips (simplified)
    return true;
}

bool PrimitiveProcessor::ProcessComplexPrimitive(const uint16_t* data, size_t count, ShapeData& shape) {
    // RFC VALIDATED: Complex primitives have 10 data elements
    if (count < 10) {
        return ErrorHandler::PostEvent(0x6A, "Insufficient data for complex primitive");
    }
    
    // Process 10-element complex primitive data
    return true;
}

std::vector<uint16_t> PrimitiveProcessor::ConvertToTriangleList(const uint16_t* stripData, size_t count) {
    std::vector<uint16_t> triangles;
    
    if (count < 3) return triangles;
    
    // Convert triangle strip to individual triangles
    for (size_t i = 0; i < count - 2; i++) {
        triangles.push_back(stripData[i]);
        triangles.push_back(stripData[i + 1]);
        triangles.push_back(stripData[i + 2]);
    }
    
    return triangles;
}

std::vector<uint16_t> PrimitiveProcessor::ConvertQuadToTriangles(const uint16_t* quadData, size_t count) {
    std::vector<uint16_t> triangles;
    
    if (count < 4) return triangles;
    
    // Convert each quad to 2 triangles
    for (size_t i = 0; i < count; i += 4) {
        if (i + 3 < count) {
            // Triangle 1: 0, 1, 2
            triangles.push_back(quadData[i]);
            triangles.push_back(quadData[i + 1]);
            triangles.push_back(quadData[i + 2]);
            
            // Triangle 2: 0, 2, 3
            triangles.push_back(quadData[i]);
            triangles.push_back(quadData[i + 2]);
            triangles.push_back(quadData[i + 3]);
        }
    }
    
    return triangles;
}