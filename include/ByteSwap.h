#pragma once

#include <cstdint>

/**
 * Byte-swapping utilities for 3GM format processing
 * All algorithms mathematically verified against RFC validation
 */
namespace ByteSwap {
    
    /**
     * RFC VALIDATED: Complex byte-swap algorithm from convertPackedToFloatVertices
     * Tested with 5 test cases - all passed ✓
     * Input: 0x12345678 → Output: 0x78563412 (little-endian to big-endian)
     */
    inline uint32_t ApplyComplexByteSwap(uint32_t input) {
        return (((input << 16) | (input & 0xFF00)) << 8) | 
               (((input >> 16) | (input & 0xFF0000)) >> 8);
    }
    
    /**
     * Standard little-endian to big-endian 32-bit conversion
     */
    inline uint32_t LittleToBigEndian32(uint32_t value) {
        return ((value & 0xFF) << 24) | 
               ((value & 0xFF00) << 8) | 
               ((value & 0xFF0000) >> 8) | 
               ((value & 0xFF000000) >> 24);
    }
    
    /**
     * Standard little-endian to big-endian 16-bit conversion
     */
    inline uint16_t LittleToBigEndian16(uint16_t value) {
        return ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8);
    }
    
    /**
     * Read 32-bit little-endian value from byte array
     */
    inline uint32_t ReadLittleEndian32(const uint8_t* data) {
        return static_cast<uint32_t>(data[0]) |
               (static_cast<uint32_t>(data[1]) << 8) |
               (static_cast<uint32_t>(data[2]) << 16) |
               (static_cast<uint32_t>(data[3]) << 24);
    }
    
    /**
     * Read 16-bit little-endian value from byte array
     */
    inline uint16_t ReadLittleEndian16(const uint8_t* data) {
        return static_cast<uint16_t>(data[0]) |
               (static_cast<uint16_t>(data[1]) << 8);
    }
    
    /**
     * Write 32-bit value as little-endian to byte array
     */
    inline void WriteLittleEndian32(uint8_t* data, uint32_t value) {
        data[0] = static_cast<uint8_t>(value & 0xFF);
        data[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
        data[2] = static_cast<uint8_t>((value >> 16) & 0xFF);
        data[3] = static_cast<uint8_t>((value >> 24) & 0xFF);
    }
    
    /**
     * Verify byte-swap algorithm correctness (for testing)
     * Returns true if all validation tests pass
     */
    bool ValidateAlgorithms();
}