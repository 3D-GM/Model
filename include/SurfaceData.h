#pragma once

#include <cstdint>
#include <vector>

/**
 * Surface data structures based on RFC validation
 * All memory layouts verified against original code
 */

/**
 * RFC VALIDATED: Surface table entry (8 bytes per surface)
 * From gm_SetSurfaceInfo.cpp lines 25-27
 */
struct SurfaceTableEntry {
    int16_t textureID;       // Offset +0: Texture identifier  
    uint16_t primitiveType;  // Offset +2: Primitive type
    uint16_t flags;          // Offset +4: Surface flags
    uint16_t status;         // Offset +6: Status and alpha flags (bit 0=active, bit 1=alpha)
    
    SurfaceTableEntry() : textureID(-1), primitiveType(0), flags(0), status(0) {}
    
    bool IsActive() const { return (status & 0x01) != 0; }
    void SetActive(bool active) { 
        if (active) status |= 0x01; 
        else status &= ~0x01; 
    }
    
    bool HasAlpha() const { return (status & 0x02) != 0; }
    void SetAlpha(bool alpha) {
        if (alpha) status |= 0x02;
        else status &= ~0x02;
    }
};

/**
 * RFC VALIDATED: Hash collision entry (16 bytes per entry)
 * From GetSurfaceHash.cpp lines 29, 31, 35
 */
struct SurfaceHashEntry {
    uint32_t searchKey;      // Offset +0: (primitiveType << 16) | flags
    uint16_t surfaceID;      // Offset +4: Surface identifier  
    uint16_t padding;        // Offset +6: Alignment padding
    int32_t nextEntry;       // Offset +8: Next entry in collision chain (-1 = end)
    uint32_t reserved;       // Offset +12: Reserved space
    
    SurfaceHashEntry() : searchKey(0), surfaceID(0), padding(0), nextEntry(-1), reserved(0) {}
    
    uint32_t GetSearchKey(uint16_t primitiveType, uint16_t flags) const {
        return (static_cast<uint32_t>(primitiveType) << 16) | flags;
    }
    
    void SetSearchKey(uint16_t primitiveType, uint16_t flags) {
        searchKey = GetSearchKey(primitiveType, flags);
    }
    
    bool IsEndOfChain() const { return nextEntry == -1; }
};

/**
 * Surface rendering data for complex shapes
 * Contains batched rendering information
 */
struct SurfaceData {
    uint16_t surfaceID;              // Surface identifier
    SurfaceTableEntry tableEntry;   // Surface properties
    
    // Rendering data
    std::vector<uint16_t> indexBuffer;     // Vertex indices for this surface
    std::vector<uint32_t> primitiveData;   // Primitive data
    
    // Batch information
    uint32_t vertexOffset;          // Offset in vertex buffer
    uint32_t indexOffset;           // Offset in index buffer
    uint32_t primitiveCount;        // Number of primitives
    
    SurfaceData() : surfaceID(0), vertexOffset(0), indexOffset(0), primitiveCount(0) {}
    
    bool IsValid() const {
        return tableEntry.IsActive() && primitiveCount > 0;
    }
};