#pragma once

#include <vector>
#include <memory>
#include <cstdint>

// Forward declarations
struct VertexData;
struct SurfaceData;
struct AnimationData;

// Primitive types for OBJ export compatibility
enum PrimitiveType {
    PRIMITIVE_TRIANGLE = 0,
    PRIMITIVE_TRIANGLE_STRIP = 1,
    PRIMITIVE_QUAD_STRIP = 2,
    PRIMITIVE_LINE_STRIP = 3,
    PRIMITIVE_POINT_SPRITE = 4,
    PRIMITIVE_TRIANGLE_LIST = 5,
    PRIMITIVE_COMPLEX = 30733
};

// Primitive data structure for OBJ export
struct PrimitiveData {
    PrimitiveType type;
    uint32_t indexCount;
    uint32_t* indices;
    int materialID;
    int textureID;
    uint16_t flags;
};

/**
 * Main container for 3D shape data
 * Based on validated RFC analysis of original ShapeData structure
 */
class ShapeData {
private:
    // Vertex Data (RFC validated: 8 floats per vertex)
    std::vector<float> vertexBuffer_;       // Raw vertex coordinates
    size_t vertexCount_;                    // Number of vertices
    
    // Primitive Data  
    std::vector<uint16_t> primitiveBuffer_; // Primitive indices
    size_t primitiveCount_;                 // Number of primitives
    
    // Surface Data (for complex rendering)
    std::vector<std::unique_ptr<SurfaceData>> surfaces_;
    
    // Animation Data (soPF chunks)
    std::unique_ptr<AnimationData> animationData_;
    
    // Shape metadata
    uint32_t shapeFlags_;                   // Processing flags (bit 3 = Line-processed, bit 7 = animated)
    int16_t textureId_;                     // Primary texture ID
    float boundingBox_[6];                  // Min/Max XYZ coordinates
    
    // Memory management
    bool isInitialized_;
    
public:
    ShapeData();
    ~ShapeData();
    
    // Vertex Buffer Management
    void AllocateVertexBuffer(size_t vertexCount);
    float* GetVertexBuffer() { return vertexBuffer_.data(); }
    const float* GetVertexBuffer() const { return vertexBuffer_.data(); }
    void SetVertexCount(size_t count) { vertexCount_ = count; }
    size_t GetVertexCount() const { return vertexCount_; }
    
    // Primitive Buffer Management  
    void AllocatePrimitiveBuffer(size_t primitiveCount);
    uint16_t* GetPrimitiveBuffer() { return primitiveBuffer_.data(); }
    const uint16_t* GetPrimitiveBuffer() const { return primitiveBuffer_.data(); }
    void SetPrimitiveCount(size_t count) { primitiveCount_ = count; }
    size_t GetPrimitiveCount() const { return primitiveCount_; }
    
    // Surface Management
    void AddSurface(std::unique_ptr<SurfaceData> surface);
    size_t GetSurfaceCount() const { return surfaces_.size(); }
    const SurfaceData* GetSurface(size_t index) const;
    
    // Animation System
    void SetAnimationData(std::unique_ptr<AnimationData> animData);
    const AnimationData* GetAnimationData() const { return animationData_.get(); }
    bool HasAnimation() const { return animationData_ != nullptr; }
    
    // Shape Properties
    void SetShapeFlags(uint32_t flags) { shapeFlags_ = flags; }
    uint32_t GetShapeFlags() const { return shapeFlags_; }
    bool IsLineProcessed() const { return (shapeFlags_ & 0x08) != 0; }  // RFC validated bit 3
    bool IsAnimated() const { return (shapeFlags_ & 0x80) != 0; }       // RFC validated bit 7
    
    void SetTextureId(int16_t id) { textureId_ = id; }
    int16_t GetTextureId() const { return textureId_; }
    
    void SetBoundingBox(const float minMax[6]);
    const float* GetBoundingBox() const { return boundingBox_; }
    
    // Validation
    bool IsValid() const;
    void Reset();
    
    // Debug output
    void PrintDebugInfo() const;
    
    // OBJ Export Compatibility Interface
    uint32_t vertexCount = 0;
    uint32_t primitiveCount = 0;
    uint32_t surfaceCount = 0;
    uint32_t animationFrameCount = 0;
    uint32_t vertexStride = 8;  // RFC validated: 8 floats per vertex
    bool hasAnimation = false;
    
    // Raw data pointers for OBJ export
    float* vertexData = nullptr;
    float* normalData = nullptr;
    float* textureCoordData = nullptr;
    float* vertexColorData = nullptr;
    PrimitiveData* primitiveData = nullptr;
    
    // Update compatibility data after parsing
    void UpdateExportData();
};