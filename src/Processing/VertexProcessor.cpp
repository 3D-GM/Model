#include "VertexProcessor.h"
#include "ByteSwap.h"
#include "GlobalVariables.h"
#include "ErrorHandler.h"
#include <cstring>

bool VertexProcessor::ProcessVertices(Algorithm algorithm,
                                     const uint8_t* inputData,
                                     float* outputBuffer,
                                     size_t vertexCount) {
    if (!ValidateInputData(algorithm, inputData, 0, vertexCount) || !outputBuffer) {
        return false;
    }
    
    switch (algorithm) {
        case Algorithm::PackedToFloat:
            return ConvertPackedToFloatVertices(
                reinterpret_cast<const uint32_t*>(inputData), 
                outputBuffer, 
                vertexCount);
            
        case Algorithm::PackedToFloat3Comp:
            return ConvertPackedToFloatVertices3Component(
                reinterpret_cast<const uint32_t*>(inputData), 
                outputBuffer, 
                vertexCount);
            
        case Algorithm::DecrunchDots:
            return DecrunchDotsVertices(inputData, outputBuffer, vertexCount);
            
        default:
            ErrorHandler::PostEvent(0x6A, "Unknown vertex processing algorithm");
            return false;
    }
}

bool VertexProcessor::ConvertPackedToFloatVertices(const uint32_t* packedVertices,
                                                  float* outputVertices,
                                                  size_t vertexCount) {
    if (!packedVertices || !outputVertices || vertexCount == 0) {
        return ErrorHandler::PostEvent(0x6A, "Invalid parameters for ConvertPackedToFloatVertices");
    }
    
    // RFC VALIDATED: Exact algorithm from convertPackedToFloatVertices.cpp lines 11-34
    float* v4 = outputVertices;  // Output pointer
    
    for (size_t i = 0; i < vertexCount; i++) {
        // RFC VALIDATED: Process X coordinate from current position (lines 15-16)
        uint32_t xPacked = ByteSwap::ApplyComplexByteSwap(*packedVertices);
        
        // CRITICAL: Jump input pointer forward by 3 DWORDs (line 17)
        packedVertices += 3;
        
        // Store X as float at current output position (line 18)
        *v4 = static_cast<float>(xPacked);
        
        // CRITICAL: Jump output pointer forward by 8 floats (line 19)
        v4 += 8;
        
        // RFC VALIDATED: Process Y coordinate from 2 positions BACK in input (lines 20-21)
        uint32_t yPacked = ByteSwap::ApplyComplexByteSwap(*(packedVertices - 2));
        
        // Store Y at output position 7 floats BACK (line 21)
        *(v4 - 7) = static_cast<float>(yPacked);
        
        // RFC VALIDATED: Process Z coordinate from 1 position BACK in input (lines 23-24)  
        uint32_t zPacked = ByteSwap::ApplyComplexByteSwap(*(packedVertices - 1));
        
        // Store Z at output position 6 floats BACK (line 24)
        *(v4 - 6) = static_cast<float>(zPacked);
    }
    
    // RFC VALIDATED: Add terminator (lines 27-28)
    uint32_t terminator = GlobalVariables::GetVertexTerminator();
    *v4 = *reinterpret_cast<const float*>(&terminator);
    
    return true;
}

bool VertexProcessor::ConvertPackedToFloatVertices3Component(const uint32_t* packedVertices,
                                                           float* outputVertices,
                                                           size_t vertexCount) {
    if (!packedVertices || !outputVertices || vertexCount == 0) {
        return ErrorHandler::PostEvent(0x6A, "Invalid parameters for ConvertPackedToFloatVertices3Component");
    }
    
    // RFC VALIDATED: Exact algorithm from convertPackedToFloatVertices_3Component.cpp lines 14-26
    float* v4 = outputVertices;  // Output pointer
    const uint32_t* input = packedVertices;
    
    for (size_t i = 0; i < vertexCount; i++) {
        // RFC VALIDATED: X coordinate - apply byte-swap to input[0] (line 16)
        *v4 = static_cast<float>(ByteSwap::ApplyComplexByteSwap(input[0]));
        
        // RFC VALIDATED: Y coordinate - apply byte-swap to input[1] (lines 17-19)
        v4[1] = static_cast<float>(ByteSwap::ApplyComplexByteSwap(input[1]));
        
        // RFC VALIDATED: Z coordinate - apply byte-swap to input[2] (lines 20-21)
        v4[2] = static_cast<float>(ByteSwap::ApplyComplexByteSwap(input[2]));
        
        // RFC VALIDATED: Advance pointers (lines 22-23)
        input += 3;    // Jump by 3 DWORDs (12 bytes)
        v4 += 8;       // Jump by 8 floats (32 bytes)
    }
    
    // RFC VALIDATED: Add terminator (line 27)
    uint32_t terminator = GlobalVariables::GetVertexTerminator();
    *v4 = *reinterpret_cast<const float*>(&terminator);
    
    return true;
}

bool VertexProcessor::DecrunchDotsVertices(const uint8_t* compressedData,
                                          float* outputVertices,
                                          size_t vertexCount) {
    if (!compressedData || !outputVertices || vertexCount == 0) {
        return ErrorHandler::PostEvent(0x6A, "Invalid parameters for DecrunchDotsVertices");
    }
    
    // RFC VALIDATED: Exact algorithm from DecrunchDots.cpp lines 31-63
    const uint8_t* inputPtr = compressedData;
    uint32_t* outputPtr = reinterpret_cast<uint32_t*>(outputVertices);
    
    // RFC VALIDATED: Phase 1 - Skip 6 compression parameters (lines 31-42)
    // Total: 24 bytes skipped
    inputPtr += 4;  // Skip param 1
    inputPtr += 4;  // Skip param 2
    inputPtr += 4;  // Skip param 3
    inputPtr += 4;  // Skip param 4
    inputPtr += 4;  // Skip param 5
    inputPtr += 4;  // Skip param 6
    
    // RFC VALIDATED: Phase 2 - Process each vertex (lines 43-62)
    for (size_t i = 0; i < vertexCount; i++) {
        // RFC VALIDATED: Read 3 int16 values (6 bytes) from compressed data (lines 48-52)
        inputPtr += 2;  // Skip to X coordinate (int16)
        inputPtr += 2;  // Skip to Y coordinate (int16)  
        inputPtr += 2;  // Skip to Z coordinate (int16)
        
        // RFC VALIDATED: Apply sub_4F2950 transformation (line 53)
        uint32_t localBuffer[8];  // This is v13 from original code
        uint32_t tempOutput[8];   // This is v12 from original code
        
        Sub4F2950Rearrangement(localBuffer, tempOutput);
        
        // RFC VALIDATED: Copy transformed data to output (32 bytes) (line 58)
        std::memcpy(outputPtr, tempOutput, 32);  // 0x20u = 32 bytes
        outputPtr += 8;  // Advance by 8 DWORDs (32 bytes)
    }
    
    // RFC VALIDATED: Add terminator (line 63)
    *outputPtr = GlobalVariables::GetVertexTerminator();
    
    return true;
}

void VertexProcessor::Sub4F2950Rearrangement(const uint32_t* input, uint32_t* output) {
    // RFC VALIDATED: Exact algorithm from sub_4F2950.cpp lines 6-10
    // Creates 8-DWORD output buffer from 3-DWORD input with specific rearrangement
    
    uint32_t temp[8] = {0};  // v4[8] from original
    
    // RFC VALIDATED: Rearrangement pattern (lines 7-9)
    temp[0] = input[0];      // *this
    temp[1] = input[1];      // *(this + 1) 
    temp[2] = input[2];      // *(this + 2)
    // temp[3] through temp[7] remain zero-initialized
    
    // RFC VALIDATED: Copy 32 bytes to output (line 10)
    std::memcpy(output, temp, 32);  // 0x20u = 32 bytes
}

size_t VertexProcessor::CalculateInputSize(Algorithm algorithm, size_t vertexCount) {
    switch (algorithm) {
        case Algorithm::PackedToFloat:
        case Algorithm::PackedToFloat3Comp:
            // 3 * uint32_t per vertex = 12 bytes per vertex
            return vertexCount * 12;
            
        case Algorithm::DecrunchDots:
            // 24 bytes compression parameters + 6 bytes per vertex
            return 24 + (vertexCount * 6);
            
        default:
            return 0;
    }
}

bool VertexProcessor::ValidateInputData(Algorithm algorithm,
                                       const uint8_t* inputData,
                                       size_t inputSize,
                                       size_t vertexCount) {
    if (!inputData || vertexCount == 0) {
        return false;
    }
    
    // Check if input size matches expected size for algorithm
    if (inputSize > 0) {
        size_t expectedSize = CalculateInputSize(algorithm, vertexCount);
        if (inputSize < expectedSize) {
            return false;
        }
    }
    
    return true;
}