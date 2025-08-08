#pragma once

#include <cstdint>
#include <string>

/**
 * Error handling system based on RFC validation
 * Implements ProcessEvent/PostEvent pattern from original code
 */
namespace ErrorHandler {
    
    // RFC validated error codes
    enum class ErrorCode : uint32_t {
        NullPointer         = 0x6A,     // Null pointer validation
        SystemNotInit       = 0x64,     // System not initialized
        SurfaceNotReady     = 0x960,    // Surface system not ready
        DynamicDataInvalid  = 0xF4,     // Dynamic data validation
        AnimationInvalid    = 0xF6,     // Animation data validation
        PolygonLimitExceeded = 221,     // Polygon limit exceeded
        InvalidTextureId    = 800,      // Invalid texture ID
        SurfaceLimitExceeded = 2402,    // Surface limit exceeded
        SurfaceAlreadyAlloc = 2403,     // Surface already allocated
        SurfaceNotAllocated = 2404      // Surface not allocated
    };
    
    /**
     * Global error state (matches original last_processed_event)
     */
    extern bool g_lastProcessedEvent;
    
    /**
     * Process system event
     * @param errorCode Error code to process
     * @return true if event processed successfully
     */
    bool ProcessEvent(uint32_t errorCode);
    
    /**
     * Post error event with data
     * @param errorCode Error code
     * @param data Additional error data
     * @return false (always indicates error condition)
     */
    bool PostEvent(uint32_t errorCode, int32_t data);
    
    /**
     * Post error event with message
     * @param errorCode Error code  
     * @param message Error message
     * @return false (always indicates error condition)
     */
    bool PostEvent(uint32_t errorCode, const std::string& message);
    
    /**
     * Check if last operation had error
     * @return true if error occurred
     */
    bool HasLastError();
    
    /**
     * Clear error state
     */
    void ClearError();
    
    /**
     * Get error code name for debugging
     * @param code Error code
     * @return Human-readable error name
     */
    const char* GetErrorName(uint32_t code);
    
    /**
     * Set debug mode for error reporting
     * @param enabled Enable debug output
     */
    void SetDebugMode(bool enabled);
}