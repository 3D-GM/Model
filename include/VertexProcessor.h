#pragma once

#include <cstdint>
#include <memory>

/**
 * Vertex processing algorithms based on RFC validation
 * All 3 algorithms mathematically verified against original code
 */
class VertexProcessor {
public:
    enum class Algorithm {
        PackedToFloat,        // convertPackedToFloatVertices - complex backward references
        PackedToFloat3Comp,   // convertPackedToFloatVertices_3Component - sequential
        DecrunchDots          // DecrunchDots - compression → decompression
    };
    
    /**
     * Process vertex data using specified algorithm
     * @param algorithm Which processing algorithm to use
     * @param inputData Input vertex data
     * @param outputBuffer Output buffer (8 floats per vertex)
     * @param vertexCount Number of vertices to process
     * @return true if processing succeeded
     */
    static bool ProcessVertices(Algorithm algorithm,
                               const uint8_t* inputData,
                               float* outputBuffer,
                               size_t vertexCount);
    
    /**
     * RFC VALIDATED: convertPackedToFloatVertices
     * Complex algorithm with backward references and advanced pointer arithmetic
     */
    static bool ConvertPackedToFloatVertices(const uint32_t* packedVertices,
                                            float* outputVertices,
                                            size_t vertexCount);
    
    /**
     * RFC VALIDATED: convertPackedToFloatVertices_3Component  
     * Sequential processing algorithm without backward references
     */
    static bool ConvertPackedToFloatVertices3Component(const uint32_t* packedVertices,
                                                      float* outputVertices,
                                                      size_t vertexCount);
    
    /**
     * RFC VALIDATED: DecrunchDots decompression pipeline
     * Decompresses vertex data with 6:32 expansion ratio
     */
    static bool DecrunchDotsVertices(const uint8_t* compressedData,
                                   float* outputVertices,
                                   size_t vertexCount);
    
    /**
     * Calculate required input data size for algorithm
     * @param algorithm Processing algorithm
     * @param vertexCount Number of vertices
     * @return Required input bytes
     */
    static size_t CalculateInputSize(Algorithm algorithm, size_t vertexCount);
    
    /**
     * Calculate output buffer size (always 8 floats per vertex)
     * @param vertexCount Number of vertices
     * @return Required output floats
     */
    static size_t CalculateOutputSize(size_t vertexCount) {
        return vertexCount * 8;  // RFC validated: 8 floats per vertex
    }
    
    /**
     * Validate input data for specified algorithm
     * @param algorithm Processing algorithm
     * @param inputData Input data
     * @param inputSize Input data size
     * @param vertexCount Expected vertex count
     * @return true if input data is valid
     */
    static bool ValidateInputData(Algorithm algorithm,
                                 const uint8_t* inputData,
                                 size_t inputSize,
                                 size_t vertexCount);
    
private:
    /**
     * RFC VALIDATED: sub_4F2950 data rearrangement function
     * Used by DecrunchDots algorithm
     */
    static void Sub4F2950Rearrangement(const uint32_t* input, uint32_t* output);
};