#pragma once

#include "SurfaceData.h"
#include <cstdint>
#include <memory>
#include <vector>

/**
 * Surface generation system based on RFC validation
 * Implements hash table with collision handling exactly as in original code
 */
class SurfaceGenerator {
private:
    // RFC VALIDATED: Hash table system globals
    std::vector<int32_t> textureHashTable_;       // dword_96C1E8: texture_id → first_hash_entry  
    std::vector<SurfaceHashEntry> hashCollisionData_; // dword_96C1F0: collision chain (16 bytes/entry)
    std::vector<SurfaceTableEntry> surfaceTable_;     // Surface info storage (8 bytes/entry)
    
    // System limits and state
    int32_t maxTextures_;                          // Maximum texture ID bound
    int32_t maxSurfaces_;                          // Maximum surface ID bound
    bool systemInitialized_;                       // Surface system ready flag
    
    // Current allocation counters
    uint16_t nextSurfaceID_;                       // Next available surface ID
    uint16_t nextHashEntry_;                       // Next available hash entry
    
public:
    SurfaceGenerator();
    ~SurfaceGenerator();
    
    /**
     * Initialize surface system with specified limits
     * @param maxTextures Maximum number of textures
     * @param maxSurfaces Maximum number of surfaces  
     * @return true if initialization succeeded
     */
    bool Initialize(int32_t maxTextures = 1000, int32_t maxSurfaces = 2000);
    
    /**
     * Cleanup surface system and free memory
     */
    void Cleanup();
    
    /**
     * RFC VALIDATED: GetSurfaceHash function implementation
     * From GetSurfaceHash.cpp lines 20-36
     */
    uint16_t GetSurfaceHash(uint16_t primitiveType, int16_t textureID, uint16_t flags);
    
    /**
     * RFC VALIDATED: getOrCreateSurface function implementation  
     * From getOrCreateSurface.cpp lines 10-35
     */
    uint16_t GetOrCreateSurface(uint16_t primitiveType, int16_t textureID, uint16_t flags);
    
    /**
     * RFC VALIDATED: gm_GetNewSurface function
     * Allocates new surface ID with validation
     */
    uint16_t GetNewSurface();
    
    /**
     * RFC VALIDATED: gm_SetSurfaceInfo function implementation
     * From gm_SetSurfaceInfo.cpp lines 25-28
     */
    bool SetSurfaceInfo(uint16_t surfaceID, uint16_t primitiveType, 
                        int16_t textureID, uint16_t flags);
    
    /**
     * RFC VALIDATED: AddSurfaceHash function
     * Adds surface to hash table with collision handling
     */
    bool AddSurfaceHash(uint16_t surfaceID);
    
    /**
     * RFC VALIDATED: UpdateSurfAlphaFlag function
     * From UpdateSurfAlphaFlag.cpp - updates alpha flags based on surface properties
     */
    bool UpdateSurfaceAlphaFlag(uint16_t surfaceID);
    
    /**
     * Get surface information by ID
     */
    const SurfaceTableEntry* GetSurfaceInfo(uint16_t surfaceID) const;
    
    /**
     * Check if surface system is ready
     */
    bool IsSystemReady() const { return systemInitialized_; }
    
    /**
     * Get system statistics
     */
    struct Statistics {
        uint16_t allocatedSurfaces;
        uint16_t allocatedHashEntries; 
        int32_t maxTextures;
        int32_t maxSurfaces;
        size_t memoryUsed;
    };
    
    Statistics GetStatistics() const;
    
    /**
     * Validate surface system integrity
     */
    bool ValidateSystem() const;
    
    /**
     * Print debug information about hash table
     */
    void PrintHashTableDebug() const;
    
private:
    /**
     * Initialize surface entry to default state
     * RFC VALIDATED: From initializeSurface.cpp
     */
    void InitializeSurface(uint16_t surfaceID);
    
    /**
     * Find free hash entry slot
     */
    uint16_t FindFreeHashEntry();
    
    /**
     * Validate texture ID bounds
     */
    bool IsValidTextureID(int16_t textureID) const;
    
    /**
     * Validate surface ID bounds  
     */
    bool IsValidSurfaceID(uint16_t surfaceID) const;
};