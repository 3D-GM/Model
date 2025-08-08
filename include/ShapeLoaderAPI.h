#pragma once

#include <cstdint>
#include <vector>

// Legacy API compatibility header

namespace ShapeLoader {

struct VertexData {
    float x, y, z;
    float nx, ny, nz; // normals
    float u, v;       // texture coords
    uint32_t color;
};

namespace Conversion {
    // Function declarations for compatibility
    uint32_t ConvertPackedToFloatVertices3Component(uint32_t* input, float* output, uint32_t vertexCount);
}

namespace Memory {
    // Memory management placeholder functions
    void* AllocateFromFreeList(size_t size);
    void DeallocateToFreeList(void* ptr);
}

} // namespace ShapeLoader