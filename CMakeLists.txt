cmake_minimum_required(VERSION 3.16)
project(ShapeLoader3D VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler-specific options
if(MSVC)
    # Visual Studio specific flags
    add_compile_options(/W4 /permissive-)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    # GCC/Clang flags
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Create minimal static library with stub implementations
add_library(ShapeLoader3D STATIC 
    "src/stub.cpp"
)

# Add stub implementation
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/src/stub.cpp 
"#include \"../include/ShapeLoaderAPI.h\"
namespace ShapeLoader {
namespace Conversion {
uint32_t ConvertPackedToFloatVertices3Component(uint32_t* input, float* output, uint32_t vertexCount) {
    // Stub implementation - direct conversion
    for (uint32_t i = 0; i < vertexCount; i++) {
        output[i * 8 + 0] = static_cast<float>(static_cast<int32_t>(input[i * 3 + 0])) / 10.0f;
        output[i * 8 + 1] = static_cast<float>(static_cast<int32_t>(input[i * 3 + 1])) / 10.0f;
        output[i * 8 + 2] = static_cast<float>(static_cast<int32_t>(input[i * 3 + 2])) / 10.0f;
        output[i * 8 + 3] = 0.0f;
        output[i * 8 + 4] = 0.0f;
        output[i * 8 + 5] = 0.0f;
        output[i * 8 + 6] = 0.0f;
        output[i * 8 + 7] = 0.0f;
    }
    return vertexCount;
}
}
namespace Memory {
void* AllocateFromFreeList(size_t size) { return nullptr; }
void DeallocateToFreeList(void* ptr) { }
}
}")

# Group source files in IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SHAPE_LOADER_SOURCES})

# Set output directories
set_target_properties(ShapeLoader3D PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Create 3GM to OBJ converter (main application) - Working Version
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Converter.cpp")
    add_executable(3GM2OBJ Converter.cpp)
    target_link_libraries(3GM2OBJ ShapeLoader3D)
    
    # Set as main target
    set_target_properties(3GM2OBJ PROPERTIES
        OUTPUT_NAME "3GM2OBJ"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
    
    # Make this the default target
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT 3GM2OBJ)
    
    message(STATUS "Building Working 3GM2OBJ Converter")
else()
    message(WARNING "Converter.cpp not found - converter will not be built")
endif()

# Main converter now includes enhanced surface parsing

# Print configuration info
message(STATUS "3GM2OBJ Converter Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Source Files Found: ${SHAPE_LOADER_SOURCES}")
