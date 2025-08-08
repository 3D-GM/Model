#pragma once

#include <cstdint>

/**
 * Global variables system based on RFC validation
 * Manages runtime constants and system state
 */
namespace GlobalVariables {
    
    /**
     * Universal vertex array terminator (dword_96BD28)
     * RFC validated: Used in ALL vertex processing functions
     * Exact value determined at runtime, likely NaN or special sentinel
     */
    uint32_t GetVertexTerminator();
    
    /**
     * Primitive type flag register (dword_9668EC)
     * RFC validated flag patterns:
     * - LOBYTE: Basic primitive flag
     * - HIBYTE: Extended data flag
     * - BYTE2: Indexed data flag  
     * - LOWORD: Complex primitive flag
     */
    uint32_t GetPrimitiveFlags();
    void SetPrimitiveFlags(uint32_t flags);
    
    /**
     * Surface system globals
     */
    namespace Surface {
        // Hash table globals (RFC validated)
        extern uint32_t* g_textureHashTable;    // dword_96C1E8: texture_id → first_hash_entry
        extern uint8_t* g_hashCollisionData;    // dword_96C1F0: hash collision chain (16 bytes/entry)
        extern uint8_t* g_surfaceTable;         // Surface info storage (8 bytes/entry)
        
        // Limits
        extern int32_t g_maxTextures;           // Maximum texture ID bound
        extern int32_t g_maxSurfaces;           // Maximum surface ID bound
        
        // System state
        extern bool g_systemInitialized;       // byte_96C1F4: Surface system ready flag
    }
    
    /**
     * Debug system globals
     */
    namespace Debug {
        extern int16_t g_debugModeLevel;        // 0=off, 1=basic, 2=verbose
        extern int32_t* g_debugStackPtr;        // Debug function call stack pointer
        extern const char** g_debugFunctionNames; // Debug function name array
        extern uint32_t* g_debugStartTimes;     // Debug timing start array
        extern uint32_t* g_debugEndTimes;       // Debug timing end array
    }
    
    /**
     * Initialize all global systems
     * Must be called before using any 3GM processing functions
     */
    bool InitializeGlobals();
    
    /**
     * Cleanup all global systems
     */
    void CleanupGlobals();
    
    /**
     * Check if globals are properly initialized
     */
    bool AreGlobalsValid();
}