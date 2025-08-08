#include "GlobalVariables.h"
#include <cmath>
#include <limits>
#include <memory>
class SurfaceGenerator;  // Forward declaration

namespace GlobalVariables {

// Static storage for global variables
static uint32_t g_vertexTerminator = 0;
static uint32_t g_primitiveFlags = 0;
static bool g_globalsInitialized = false;

// Surface system globals
namespace Surface {
    uint32_t* g_textureHashTable = nullptr;
    uint8_t* g_hashCollisionData = nullptr; 
    uint8_t* g_surfaceTable = nullptr;
    int32_t g_maxTextures = 1000;  // Default limits
    int32_t g_maxSurfaces = 2000;
    bool g_systemInitialized = false;
    
    // Global surface generator instance
    static std::unique_ptr<class SurfaceGenerator> g_surfaceGenerator = nullptr;
    
    bool InitializeSurfaceSystem() {
        if (!g_surfaceGenerator) {
            g_surfaceGenerator = std::make_unique<class SurfaceGenerator>();
        }
        return g_surfaceGenerator->Initialize(g_maxTextures, g_maxSurfaces);
    }
    
    void CleanupSurfaceSystem() {
        if (g_surfaceGenerator) {
            g_surfaceGenerator->Cleanup();
            g_surfaceGenerator.reset();
        }
    }
    
    class SurfaceGenerator* GetSurfaceGenerator() {
        return g_surfaceGenerator.get();
    }
}

// Debug system globals  
namespace Debug {
    int16_t g_debugModeLevel = 0;
    int32_t* g_debugStackPtr = nullptr;
    const char** g_debugFunctionNames = nullptr;
    uint32_t* g_debugStartTimes = nullptr;
    uint32_t* g_debugEndTimes = nullptr;
}

uint32_t GetVertexTerminator() {
    if (!g_globalsInitialized) {
        InitializeGlobals();
    }
    return g_vertexTerminator;
}

uint32_t GetPrimitiveFlags() {
    return g_primitiveFlags;
}

void SetPrimitiveFlags(uint32_t flags) {
    g_primitiveFlags = flags;
}

bool InitializeGlobals() {
    if (g_globalsInitialized) {
        return true;
    }
    
    // Initialize vertex terminator
    // RFC analysis shows this is used as a sentinel value in float arrays
    // Using quiet NaN as a safe sentinel value
    float nanValue = std::numeric_limits<float>::quiet_NaN();
    g_vertexTerminator = *reinterpret_cast<uint32_t*>(&nanValue);
    
    // Initialize primitive flags to clean state
    g_primitiveFlags = 0;
    
    // Initialize surface system
    Surface::g_systemInitialized = false;  // Will be set when surface system starts
    
    // Initialize debug system
    Debug::g_debugModeLevel = 0;  // Debug off by default
    
    g_globalsInitialized = true;
    return true;
}

void CleanupGlobals() {
    // Clean up any allocated memory
    if (Surface::g_textureHashTable) {
        delete[] Surface::g_textureHashTable;
        Surface::g_textureHashTable = nullptr;
    }
    
    if (Surface::g_hashCollisionData) {
        delete[] Surface::g_hashCollisionData;
        Surface::g_hashCollisionData = nullptr;
    }
    
    if (Surface::g_surfaceTable) {
        delete[] Surface::g_surfaceTable;
        Surface::g_surfaceTable = nullptr;
    }
    
    // Reset debug system
    Debug::g_debugStackPtr = nullptr;
    Debug::g_debugFunctionNames = nullptr;
    Debug::g_debugStartTimes = nullptr;
    Debug::g_debugEndTimes = nullptr;
    
    g_globalsInitialized = false;
}

bool AreGlobalsValid() {
    return g_globalsInitialized;
}

} // namespace GlobalVariables