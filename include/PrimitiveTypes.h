#pragma once

#include <cstdint>

/**
 * 3GM Primitive Type System
 * All constants and flag patterns RFC-validated from original code
 */

/**
 * RFC VALIDATED: All 7 primitive type constants from parsePrimitiveChunk.cpp
 */
enum class PrimitiveType : uint16_t {
    TriangleStrip   = 16646,    // Triangle strip rendering
    QuadStripInput  = 18189,    // Quad strip input format (converted to 18190)
    QuadStrip       = 18190,    // Quad strip processed format
    TriangleList    = 20486,    // Triangle list rendering
    PointSprite     = 21251,    // Point sprite/billboard rendering
    LineStrip       = 28422,    // Line strip rendering
    LineStripAlt    = 28423,    // Line strip variant (converted to 21251)
    ComplexPrimitive = 30733,   // Complex primitive (10 data elements)
    
    // Control constants
    EndMarker       = 24576,    // 0x6000 - End of primitive parsing
    Terminator      = 0xFFFE    // -2 - Final primitive list terminator
};

/**
 * RFC VALIDATED: Primitive flag patterns for dword_9668EC register
 */
struct PrimitiveFlags {
    static const uint32_t LOBYTE_BASIC     = 0x00000001;  // Basic primitive flag
    static const uint32_t HIBYTE_EXTENDED  = 0x00000100;  // Extended data flag  
    static const uint32_t BYTE2_INDEXED    = 0x00010000;  // Indexed data flag
    static const uint32_t LOWORD_COMPLEX   = 0x00000101;  // Complex primitive flag (LOBYTE + HIBYTE)
    
    /**
     * Get flag pattern for primitive type
     * RFC VALIDATED from parsePrimitiveChunk.cpp switch statement
     */
    static uint32_t GetFlagsForType(PrimitiveType type) {
        switch (type) {
            case PrimitiveType::TriangleStrip:
                return LOBYTE_BASIC | BYTE2_INDEXED;    // LOBYTE=1, BYTE2=1
                
            case PrimitiveType::QuadStrip:
                return LOWORD_COMPLEX | HIBYTE_EXTENDED; // LOWORD=257, HIBYTE=1
                
            case PrimitiveType::TriangleList:
                return LOBYTE_BASIC | BYTE2_INDEXED;    // LOBYTE=1, BYTE2=1
                
            case PrimitiveType::PointSprite:
                return LOBYTE_BASIC;                    // LOBYTE=1
                
            case PrimitiveType::LineStrip:
                return LOBYTE_BASIC | HIBYTE_EXTENDED;  // LOBYTE=1, HIBYTE=1
                
            case PrimitiveType::ComplexPrimitive:
                return LOWORD_COMPLEX;                  // LOWORD=257
                
            default:
                return 0;
        }
    }
};

/**
 * RFC VALIDATED: Type conversion rules from convertChunkedDataToSurfaces.cpp
 */
struct PrimitiveTypeConverter {
    /**
     * Apply conversion rules for input primitive types
     */
    static PrimitiveType ConvertInputType(PrimitiveType inputType) {
        switch (inputType) {
            case PrimitiveType::QuadStripInput:
                return PrimitiveType::QuadStrip;        // 18189 → 18190
                
            case PrimitiveType::LineStripAlt:
                return PrimitiveType::PointSprite;      // 28423 → 21251
                
            default:
                return inputType;  // No conversion needed
        }
    }
    
    /**
     * Check if primitive type needs special handling
     */
    static bool RequiresSpecialHandling(PrimitiveType type) {
        return (type == PrimitiveType::LineStrip || 
                type == PrimitiveType::QuadStripInput ||
                type == PrimitiveType::ComplexPrimitive);
    }
    
    /**
     * Get data element count for primitive type
     */
    static int GetDataElementCount(PrimitiveType type) {
        switch (type) {
            case PrimitiveType::ComplexPrimitive:
                return 10;  // RFC validated: 10 data elements
            default:
                return -1;  // Variable element count
        }
    }
};

/**
 * Utility functions for primitive type handling
 */
namespace PrimitiveUtils {
    
    /**
     * Convert raw uint16_t to PrimitiveType enum
     */
    inline PrimitiveType FromRawValue(uint16_t value) {
        return static_cast<PrimitiveType>(value);
    }
    
    /**
     * Convert PrimitiveType to raw uint16_t
     */
    inline uint16_t ToRawValue(PrimitiveType type) {
        return static_cast<uint16_t>(type);
    }
    
    /**
     * Check if primitive type is valid/known
     */
    bool IsValidPrimitiveType(PrimitiveType type);
    
    /**
     * Get human-readable name for primitive type
     */
    const char* GetTypeName(PrimitiveType type);
    
    /**
     * Check if primitive type is a control constant
     */
    bool IsControlConstant(PrimitiveType type);
}