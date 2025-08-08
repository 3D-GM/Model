#pragma once

#include <cstdint>

/**
 * 3GM Chunk Types - All values in little-endian format
 * Based on validated RFC analysis of original reverse-engineered code
 */
enum class ChunkType : uint32_t {
    // Vertex Data Chunks
    Dot2 = 0x32746f44,  // "Dot2" - Vertex coordinates (original format)
    FDot = 0x746f4446,  // "FDot" - Compressed vertex data 
    
    // Primitive Data Chunks  
    Prim = 0x6d697250,  // "Prim" - Simple primitives
    Line = 0x656e694c,  // "Line" - Complex primitives (4-phase processing)
    
    // Animation Chunks
    soPF = 0x46506f73,  // "soPF" - Animation position keyframes
    FPos = 0x736f5046,  // "FPos" - Position data companion
    
    // Metadata Chunks
    TxNm = 0x6d4e7854,  // "TxNm" - Texture names
    
    // Control Chunks
    End  = 0x20646e45,  // "End " - File terminator (note trailing space)
    
    // Unknown/Unsupported
    Unknown = 0x00000000
};

/**
 * Convert raw 4-byte chunk ID to ChunkType enum
 * Handles little-endian byte order conversion
 */
inline ChunkType GetChunkTypeFromRawID(uint32_t rawID) {
    switch (rawID) {
        case 0x32746f44: return ChunkType::Dot2;
        case 0x746f4446: return ChunkType::FDot; 
        case 0x6d697250: return ChunkType::Prim;
        case 0x656e694c: return ChunkType::Line;
        case 0x46506f73: return ChunkType::soPF;
        case 0x736f5046: return ChunkType::FPos;
        case 0x6d4e7854: return ChunkType::TxNm;
        case 0x20646e45: return ChunkType::End;
        default:         return ChunkType::Unknown;
    }
}

/**
 * Convert ChunkType to human-readable string for debugging
 */
inline const char* ChunkTypeToString(ChunkType type) {
    switch (type) {
        case ChunkType::Dot2: return "Dot2";
        case ChunkType::FDot: return "FDot";
        case ChunkType::Prim: return "Prim";
        case ChunkType::Line: return "Line";
        case ChunkType::soPF: return "soPF";
        case ChunkType::FPos: return "FPos";
        case ChunkType::TxNm: return "TxNm";
        case ChunkType::End:  return "End";
        case ChunkType::Unknown: return "Unknown";
        default: return "Invalid";
    }
}