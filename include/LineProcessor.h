#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

/**
 * RFC VALIDATED: Line Chunk 4-Phase Processing Pipeline
 * 
 * Line chunks use a completely different processing algorithm than Prim chunks.
 * This implements the complex 4-phase pipeline from convertChunkedDataToSurfaces.cpp:
 * 
 * Phase 1: Line segment reading with byte swapping
 * Phase 2: Special primitive type conversions (LineStrip→PointSprite, etc.)  
 * Phase 3: Line data processing with termination markers
 * Phase 4: Complex primitive surface creation
 */
class LineProcessor {
public:
    /**
     * Primitive conversion tracking for Phase 2
     */
    struct PrimitiveConversion {
        uint16_t originalType;
        uint16_t convertedType;
    };
    
    /**
     * Processing state for 4-phase pipeline
     */
    struct LineProcessingState {
        const uint16_t* inputPtr;           // Current input position
        const uint16_t* inputEnd;           // End of input data
        uint32_t* outputPtr;                // Current output position
        uint16_t currentPrimitiveType;      // Active primitive type
        
        // RFC VALIDATED: Buffer sizes from original (v23[18], v24[18])
        std::vector<uint32_t> primitiveDataBuffer;    // v24[18] - primitive extraction
        std::vector<uint32_t> complexPrimitiveBuffer; // v23[18] - complex primitive data
        
        // Phase 2 conversion tracking
        std::vector<PrimitiveConversion> primitiveConversions;
    };
    
public:
    LineProcessor();
    ~LineProcessor();
    
    /**
     * Process Line chunk using 4-phase pipeline
     * @param chunkData Raw chunk data
     * @param chunkSize Size of chunk data
     * @param debugName Optional name for debugging
     * @return true if processing succeeded
     */
    bool ProcessLineChunk(const uint8_t* chunkData, size_t chunkSize, 
                         const std::string& debugName = "");
    
    /**
     * Check if chunk type represents a Line chunk
     */
    static bool IsLineChunk(uint32_t chunkType);
    
    /**
     * Estimate output buffer size needed for Line chunk
     */
    static size_t EstimateOutputSize(size_t inputSize);
    
private:
    // RFC VALIDATED: 4-Phase Processing Pipeline
    
    /**
     * Initialize processing state from input data
     */
    bool InitializeProcessingState(LineProcessingState& state, 
                                  const uint8_t* data, size_t size);
    
    /**
     * Phase 1: Read line segments with byte swapping
     * RFC: Lines 30-47 of convertChunkedDataToSurfaces.cpp
     */
    bool Phase1_ReadLineSegments(LineProcessingState& state);
    
    /**
     * Phase 2: Convert special primitive types
     * RFC: Lines 48-66 - LineStrip→PointSprite, QuadStrip conversions
     */
    bool Phase2_ConvertPrimitiveTypes(LineProcessingState& state);
    
    /**
     * Phase 3: Process line data with termination
     * RFC: Lines 68-86 - Handle line data until 0x7000 terminator
     */
    bool Phase3_ProcessLineData(LineProcessingState& state);
    
    /**
     * Phase 4: Create complex primitive surfaces  
     * RFC: Lines 87-111 - Handle type 17165 complex primitives
     */
    bool Phase4_CreateComplexPrimitiveSurfaces(LineProcessingState& state);
    
    // Helper functions
    
    /**
     * Handle special line types in Phase 1
     * RFC: Lines 48-66 - Extract and convert primitives
     */
    bool HandleSpecialLineType(LineProcessingState& state);
    
    /**
     * Extract primitive data from source buffer
     * RFC: Based on extractPrimitiveData.cpp pattern
     */
    bool ExtractPrimitiveData(uint32_t* sourceData, uint32_t* targetBuffer, 
                             uint32_t extractionMode);
    
    /**
     * Create surface from primitive data
     * RFC: Based on createSurfaceFromPrimitive.cpp pattern
     */
    bool CreateSurfaceFromPrimitive(uint32_t* primitiveData, uint32_t* surfaceData);
    
    /**
     * Prepare output buffer with estimated size
     */
    bool PrepareOutputBuffer(size_t estimatedSize);
    
    /**
     * Finalize output with termination markers
     */
    bool FinalizeOutput(LineProcessingState& state);
    
    /**
     * Cleanup allocated buffers
     */
    void CleanupBuffers();
    
private:
    // Buffer management
    uint8_t* outputBuffer_;
    size_t bufferSize_;
};