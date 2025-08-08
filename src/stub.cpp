#include "../include/ShapeLoaderAPI.h"
namespace ShapeLoader {
namespace Conversion {
uint32_t ConvertPackedToFloatVertices3Component(uint32_t* input, float* output, uint32_t vertexCount) {
    // Stub implementation - direct conversion
    for (uint32_t i = 0; i < vertexCount; i++) {
        output[i * 8 + 0] = static_cast<float>(static_cast<int32_t>(input[i * 3 + 0])) / 10.0f;
        output[i * 8 + 1] = static_cast<float>(static_cast<int32_t>(input[i * 3 + 1])) / 10.0f;
        output[i * 8 + 2] = static_cast<float>(static_cast<int32_t>(input[i * 3 + 2])) / 10.0f;
        output[i * 8 + 3] = 0.0f;
        output[i * 8 + 4] = 0.0f;
        output[i * 8 + 5] = 0.0f;
        output[i * 8 + 6] = 0.0f;
        output[i * 8 + 7] = 0.0f;
    }
    return vertexCount;
}
}
namespace Memory {
void* AllocateFromFreeList(size_t size) { return nullptr; }
void DeallocateToFreeList(void* ptr) { }
}
}