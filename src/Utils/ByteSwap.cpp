#include "ByteSwap.h"
#include <iostream>
#include <iomanip>

namespace ByteSwap {

bool ValidateAlgorithms() {
    // RFC validated test cases
    struct TestCase {
        uint32_t input;
        uint32_t expected;
        const char* description;
    };
    
    TestCase testCases[] = {
        {0x12345678, 0x78563412, "Standard test"},
        {0x01020304, 0x04030201, "Sequential bytes"},
        {0xFF00FF00, 0x00FF00FF, "Alternating pattern"},
        {0x00000000, 0x00000000, "Zero"},
        {0xFFFFFFFF, 0xFFFFFFFF, "All ones"}
    };
    
    std::cout << "ByteSwap Algorithm Validation:\n";
    std::cout << "Input      -> Output     | Expected   | Status\n";
    std::cout << "------------------------------------------------\n";
    
    bool allPassed = true;
    
    for (const auto& test : testCases) {
        uint32_t result = ApplyComplexByteSwap(test.input);
        bool passed = (result == test.expected);
        allPassed &= passed;
        
        std::cout << "0x" << std::hex << std::setfill('0') << std::setw(8) << test.input 
                  << " -> 0x" << std::setw(8) << result 
                  << " | 0x" << std::setw(8) << test.expected
                  << " | " << (passed ? "✓" : "✗") << " " << test.description << "\n";
    }
    
    std::cout << std::dec;  // Reset to decimal
    
    if (allPassed) {
        std::cout << "✅ All byte-swap algorithms VERIFIED\n";
    } else {
        std::cout << "❌ Byte-swap validation FAILED\n";
    }
    
    return allPassed;
}

} // namespace ByteSwap