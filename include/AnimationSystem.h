#pragma once

#include "AnimationData.h"
#include <cstdint>
#include <vector>
#include <memory>

/**
 * RFC VALIDATED: Animation System Controller
 * Based on gm_ApplyShapeKeyFrames.cpp and related animation functions
 * 
 * This class manages the complete animation pipeline including:
 * - Keyframe interpolation (interpolateBatchKeyframe.cpp)
 * - Batch time management (setBatchTime.cpp, setBatchTimeRecursive.cpp)
 * - Shape keyframe application (gm_ApplyShapeKeyFrames.cpp)
 * - soPF and FPos chunk processing
 */
class AnimationSystem {
public:
    AnimationSystem();
    ~AnimationSystem();
    
    /**
     * Initialize animation system with specified limits
     * @param maxBatches Maximum number of animation batches
     * @param maxKeyframes Maximum number of keyframes
     * @return true if initialization succeeded
     */
    bool Initialize(uint32_t maxBatches = 1000, uint32_t maxKeyframes = 10000);
    
    /**
     * Cleanup animation system and free memory
     */
    void Cleanup();
    
    /**
     * RFC VALIDATED: Apply shape keyframes to shape data
     * From gm_ApplyShapeKeyFrames.cpp lines 37-151
     */
    bool ApplyShapeKeyFrames(uint32_t shapeData);
    
    /**
     * RFC VALIDATED: Set batch time for animation
     * From setBatchTime.cpp lines 22-55
     */
    bool SetBatchTime(uint32_t shapeData, int32_t batchIndex, float time, bool recursive);
    
    /**
     * RFC VALIDATED: Interpolate keyframe data between batches
     * From interpolateBatchKeyframe.cpp lines 15-71
     */
    bool InterpolateBatchKeyframe(uint32_t batchIndex, InterpolationResult& result);
    
    /**
     * Process soPF (Shape Object Property Frame) chunk
     * @param chunkData Raw chunk data
     * @param chunkSize Size of chunk data
     * @return true if processing succeeded
     */
    bool ProcessSoPFChunk(const uint8_t* chunkData, size_t chunkSize);
    
    /**
     * Process FPos (Frame Position) chunk
     * @param chunkData Raw chunk data  
     * @param chunkSize Size of chunk data
     * @return true if processing succeeded
     */
    bool ProcessFPosChunk(const uint8_t* chunkData, size_t chunkSize);
    
    /**
     * Update animation system for current frame
     * @param deltaTime Time elapsed since last frame
     */
    void UpdateAnimations(float deltaTime);
    
    /**
     * Check if animation system is ready
     */
    bool IsSystemReady() const { return systemInitialized_; }
    
    /**
     * Get animation system statistics
     */
    struct Statistics {
        uint32_t activeBatches;
        uint32_t totalKeyframes;
        float currentTime;
        size_t memoryUsed;
        uint32_t interpolationsPerFrame;
    };
    
    Statistics GetStatistics() const;
    
    /**
     * Set global animation time scale
     */
    void SetTimeScale(float scale);
    
    /**
     * Get current global animation time
     */
    float GetGlobalTime() const;
    
    /**
     * Enable/disable debug mode
     */
    void SetDebugMode(bool enable);
    
    /**
     * Validate animation system integrity
     */
    bool ValidateSystem() const;
    
    /**
     * Print debug information about animation state
     */
    void PrintAnimationDebug() const;
    
private:
    // Core animation processing functions
    
    /**
     * RFC VALIDATED: Set batch time recursively
     * From setBatchTimeRecursive.cpp pattern
     */
    bool SetBatchTimeRecursive(uint32_t batchData, float time);
    
    /**
     * RFC VALIDATED: Transform batch vertices
     * From transformBatchVertices.cpp pattern (referenced in gm_ApplyShapeKeyFrames.cpp:121)
     */
    bool TransformBatchVertices(uint32_t batchIndex);
    
    /**
     * RFC VALIDATED: Vertex transformation from keyframe
     * From transformVertexFromKeyFrame.cpp pattern (referenced in line 139)
     */
    bool TransformVertexFromKeyFrame(uint32_t batchIndex, uint32_t targetBatch, 
                                   uint32_t dataOffset, float interpolationFactor);
    
    /**
     * Find keyframe by time
     */
    int32_t FindKeyframeAtTime(uint32_t batchIndex, float time);
    
    /**
     * Initialize animation batch
     */
    bool InitializeAnimationBatch(uint32_t batchIndex);
    
    /**
     * Process animation chunk data
     */
    bool ProcessAnimationChunkData(const uint8_t* data, size_t size, 
                                  SoPFChunkData& sopfData);
    
    /**
     * Process frame position data
     */
    bool ProcessFramePositionData(const uint8_t* data, size_t size,
                                 FPosChunkData& fposData);
    
    /**
     * Update batch animation state
     */
    void UpdateBatchState(AnimationBatch& batch, float deltaTime);
    
    /**
     * Validate animation batch data
     */
    bool ValidateBatch(const AnimationBatch& batch) const;
    
private:
    // RFC VALIDATED: System state matching original global variables
    AnimationSystemGlobals globals_;           // Animation system globals
    std::vector<AnimationBatch> batches_;      // Animation batches
    std::vector<KeyframeData> keyframes_;      // All keyframe data
    std::vector<SoPFChunkData> sopfChunks_;    // soPF chunk data
    std::vector<FPosChunkData> fposChunks_;    // FPos chunk data
    
    // System configuration
    uint32_t maxBatches_;                      // Maximum batch count
    uint32_t maxKeyframes_;                    // Maximum keyframe count
    bool systemInitialized_;                   // System ready flag
    
    // Performance tracking
    uint32_t frameInterpolations_;             // Interpolations this frame
    float lastUpdateTime_;                     // Last update timestamp
};