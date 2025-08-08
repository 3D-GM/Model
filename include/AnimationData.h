#pragma once

#include <cstdint>
#include <vector>
#include <memory>

/**
 * RFC VALIDATED: Animation Data Structures
 * Based on analysis of gm_ApplyShapeKeyFrames.cpp and related animation functions
 */

/**
 * Keyframe data structure for animation interpolation
 * RFC: Based on interpolateBatchKeyframe.cpp lines 48-61
 */
struct KeyframeData {
    float time;           // Keyframe time stamp (v12, v14 from original)
    uint32_t batchID;     // Batch identifier (v13 from original)
    
    KeyframeData() : time(0.0f), batchID(0) {}
    KeyframeData(float t, uint32_t id) : time(t), batchID(id) {}
    
    bool operator<(const KeyframeData& other) const {
        return time < other.time;
    }
};

/**
 * Animation batch data structure
 * RFC: Based on setBatchTime.cpp and gm_ApplyShapeKeyFrames.cpp structure analysis
 * Each batch is 204 bytes (lines 15, 35-36 from setBatchTime.cpp)
 */
struct AnimationBatch {
    static constexpr size_t BATCH_SIZE = 204; // RFC VALIDATED: 204 bytes per batch
    
    uint32_t batchID;                      // Batch identifier
    float currentTime;                     // Current animation time (offset +116)
    float targetTime;                      // Target time for interpolation (offset +120)
    uint32_t keyframeCount;                // Number of keyframes (offset +108)
    uint32_t keyframeOffset;               // Offset to keyframe data (offset +112)
    
    // Transformation data (offsets from original analysis)
    uint32_t childBatch;                   // Child batch pointer (offset +68)
    uint32_t renderDataPtr;                // Render batch data pointer
    
    // Animation state
    bool isActive;                         // Animation active flag
    bool requiresUpdate;                   // Update required flag
    
    AnimationBatch() 
        : batchID(0), currentTime(0.0f), targetTime(0.0f), 
          keyframeCount(0), keyframeOffset(0), childBatch(0), 
          renderDataPtr(0), isActive(false), requiresUpdate(false) {}
};

/**
 * Shape animation keyframe structure
 * RFC: Based on interpolateBatchKeyframe.cpp keyframe processing
 */
struct ShapeKeyframe {
    float time;                            // Keyframe timestamp
    uint32_t dataOffset;                   // Offset to keyframe data
    
    // Interpolation data
    uint32_t prevBatch;                    // Previous batch ID (v11 from original)
    uint32_t nextBatch;                    // Next batch ID (v9 from original)
    float interpolationFactor;             // Interpolation factor (0.0-1.0)
    
    ShapeKeyframe() 
        : time(0.0f), dataOffset(0), prevBatch(0), 
          nextBatch(0), interpolationFactor(0.0f) {}
};

/**
 * Animation system global state
 * RFC: Based on global variables from gm_ApplyShapeKeyFrames.cpp
 */
struct AnimationSystemGlobals {
    // RFC VALIDATED: Global animation variables from original code
    uint32_t currentShapeData;             // dword_96C828 - current shape data pointer
    uint32_t currentBatchData;             // dword_96C830 - current batch data pointer  
    uint32_t currentSurfaceData;           // dword_96C834 - current surface data pointer
    uint32_t currentRenderBatch;           // dword_96C87C - current render batch pointer
    float globalAnimationTime;             // dword_96C82C - global animation time
    float timeScale;                       // flt_96C824 - time scaling factor
    
    // Animation state flags
    bool systemInitialized;                // Animation system ready
    bool debugMode;                        // Debug mode active
    
    AnimationSystemGlobals() 
        : currentShapeData(0), currentBatchData(0), currentSurfaceData(0),
          currentRenderBatch(0), globalAnimationTime(0.0f), timeScale(1.0f),
          systemInitialized(false), debugMode(false) {}
};

/**
 * Animation processing context
 * Used during keyframe application and interpolation
 */
struct AnimationContext {
    AnimationSystemGlobals* globals;       // System global state
    std::vector<AnimationBatch>* batches;  // Active animation batches
    std::vector<KeyframeData>* keyframes;  // Keyframe data array
    
    // Processing state
    uint32_t currentBatchIndex;            // Currently processing batch
    float deltaTime;                       // Time delta for this frame
    bool forceUpdate;                      // Force update all batches
    
    AnimationContext() 
        : globals(nullptr), batches(nullptr), keyframes(nullptr),
          currentBatchIndex(0), deltaTime(0.0f), forceUpdate(false) {}
};

/**
 * Animation interpolation result
 * RFC: Based on interpolateBatchKeyframe.cpp output parameters
 */
struct InterpolationResult {
    uint32_t fromBatch;                    // Source batch ID (a2 from original)
    uint32_t toBatch;                      // Target batch ID (a3 from original)  
    float interpolationFactor;             // Blend factor (a4 from original)
    bool isStaticFrame;                    // No interpolation needed
    
    InterpolationResult() 
        : fromBatch(0), toBatch(0), interpolationFactor(0.0f), 
          isStaticFrame(true) {}
};

/**
 * soPF chunk data structure
 * Shape Object Property Frame - contains animation properties
 */
struct SoPFChunkData {
    uint32_t shapeID;                      // Shape identifier
    uint32_t propertyCount;                // Number of animated properties
    float timeStamp;                       // Frame timestamp
    uint32_t dataSize;                     // Size of property data
    
    // Property data follows (variable size)
    std::vector<uint8_t> propertyData;
    
    SoPFChunkData() 
        : shapeID(0), propertyCount(0), timeStamp(0.0f), dataSize(0) {}
};

/**
 * FPos chunk data structure  
 * Frame Position - contains keyframe position data
 */
struct FPosChunkData {
    uint32_t frameCount;                   // Number of frames
    float startTime;                       // Animation start time
    float endTime;                         // Animation end time
    uint32_t positionDataSize;             // Size of position data
    
    // Position data follows (variable size)
    std::vector<float> positionData;
    
    FPosChunkData() 
        : frameCount(0), startTime(0.0f), endTime(0.0f), positionDataSize(0) {}
};

/**
 * Legacy animation data structure (for compatibility)
 */
struct AnimationData {
    uint32_t keyframeCount;              // Number of keyframes
    std::vector<float> keyframeBuffer;   // Keyframe data buffer
    uint32_t bufferSize;                 // Total buffer size
    
    AnimationData() : keyframeCount(0), bufferSize(0) {}
};