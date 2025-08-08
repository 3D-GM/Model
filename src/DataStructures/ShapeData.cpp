#include "ShapeData.h"
#include "SurfaceData.h"
#include "AnimationData.h"
#include <iostream>
#include <cstring>

ShapeData::ShapeData() 
    : vertexCount_(0)
    , primitiveCount_(0)
    , shapeFlags_(0)
    , textureId_(-1)
    , isInitialized_(false) {
    
    // Initialize bounding box to invalid state
    std::memset(boundingBox_, 0, sizeof(boundingBox_));
}

ShapeData::~ShapeData() {
    Reset();
}

void ShapeData::AllocateVertexBuffer(size_t vertexCount) {
    // RFC validated: 8 floats per vertex (X,Y,Z + 5 additional data)
    size_t floatCount = vertexCount * 8;
    vertexBuffer_.resize(floatCount);
    vertexCount_ = vertexCount;
}

void ShapeData::AllocatePrimitiveBuffer(size_t primitiveCount) {
    // Allocate primitive index buffer
    primitiveBuffer_.resize(primitiveCount);
    primitiveCount_ = primitiveCount;
}

void ShapeData::AddSurface(std::unique_ptr<SurfaceData> surface) {
    if (surface) {
        surfaces_.push_back(std::move(surface));
    }
}

const SurfaceData* ShapeData::GetSurface(size_t index) const {
    if (index >= surfaces_.size()) {
        return nullptr;
    }
    return surfaces_[index].get();
}

void ShapeData::SetAnimationData(std::unique_ptr<AnimationData> animData) {
    animationData_ = std::move(animData);
    if (animationData_) {
        shapeFlags_ |= 0x80;  // Set animation flag (RFC validated bit 7)
    }
}

void ShapeData::SetBoundingBox(const float minMax[6]) {
    std::memcpy(boundingBox_, minMax, sizeof(boundingBox_));
}

bool ShapeData::IsValid() const {
    // Basic validation checks
    if (vertexCount_ == 0) return false;
    if (vertexBuffer_.size() != vertexCount_ * 8) return false;  // RFC validated: 8 floats per vertex
    if (textureId_ < -1) return false;  // -1 is valid (no texture)
    
    return true;
}

void ShapeData::Reset() {
    vertexBuffer_.clear();
    primitiveBuffer_.clear();
    surfaces_.clear();
    animationData_.reset();
    
    vertexCount_ = 0;
    primitiveCount_ = 0;
    shapeFlags_ = 0;
    textureId_ = -1;
    std::memset(boundingBox_, 0, sizeof(boundingBox_));
    isInitialized_ = false;
}

void ShapeData::PrintDebugInfo() const {
    std::cout << "ShapeData Debug Info:\n";
    std::cout << "  Vertices: " << vertexCount_ << "\n";
    std::cout << "  Primitives: " << primitiveCount_ << "\n";
    std::cout << "  Surfaces: " << surfaces_.size() << "\n";
    std::cout << "  Texture ID: " << textureId_ << "\n";
    std::cout << "  Flags: 0x" << std::hex << shapeFlags_ << std::dec << "\n";
    std::cout << "  Line Processed: " << (IsLineProcessed() ? "Yes" : "No") << "\n";
    std::cout << "  Animated: " << (IsAnimated() ? "Yes" : "No") << "\n";
    std::cout << "  Bounding Box: [" 
              << boundingBox_[0] << "," << boundingBox_[1] << "," << boundingBox_[2] << "] to ["
              << boundingBox_[3] << "," << boundingBox_[4] << "," << boundingBox_[5] << "]\n";
}