#include "ErrorHandler.h"
#include <iostream>
#include <iomanip>

namespace ErrorHandler {

// Global error state (matches original last_processed_event)
bool g_lastProcessedEvent = false;

// Debug mode flag
static bool g_debugMode = false;

bool ProcessEvent(uint32_t errorCode) {
    if (g_debugMode) {
        std::cout << "[ProcessEvent] Code: 0x" << std::hex << errorCode 
                  << " (" << GetErrorName(errorCode) << ")" << std::dec << std::endl;
    }
    
    // In original code, ProcessEvent could succeed or fail
    // For our implementation, we'll assume it succeeds unless it's a critical error
    switch (errorCode) {
        case 0x6A:   // Null pointer - critical
        case 0x64:   // System not initialized - critical  
            g_lastProcessedEvent = true;
            return false;
            
        default:     // Other events can be processed
            return true;
    }
}

bool PostEvent(uint32_t errorCode, int32_t data) {
    g_lastProcessedEvent = true;
    
    if (g_debugMode) {
        std::cout << "[PostEvent] Code: 0x" << std::hex << errorCode 
                  << " (" << GetErrorName(errorCode) << "), Data: " << std::dec << data << std::endl;
    }
    
    return false;  // PostEvent always indicates error
}

bool PostEvent(uint32_t errorCode, const std::string& message) {
    g_lastProcessedEvent = true;
    
    if (g_debugMode) {
        std::cout << "[PostEvent] Code: 0x" << std::hex << errorCode 
                  << " (" << GetErrorName(errorCode) << "), Message: " << std::dec 
                  << message << std::endl;
    }
    
    return false;  // PostEvent always indicates error
}

bool HasLastError() {
    return g_lastProcessedEvent;
}

void ClearError() {
    g_lastProcessedEvent = false;
}

const char* GetErrorName(uint32_t code) {
    switch (code) {
        case 0x6A:   return "NullPointer";
        case 0x64:   return "SystemNotInit";
        case 0x960:  return "SurfaceNotReady";
        case 0xF4:   return "DynamicDataInvalid";
        case 0xF6:   return "AnimationInvalid";
        case 221:    return "PolygonLimitExceeded";
        case 800:    return "InvalidTextureId";
        case 2402:   return "SurfaceLimitExceeded";
        case 2403:   return "SurfaceAlreadyAlloc";
        case 2404:   return "SurfaceNotAllocated";
        default:     return "Unknown";
    }
}

void SetDebugMode(bool enabled) {
    g_debugMode = enabled;
}

} // namespace ErrorHandler