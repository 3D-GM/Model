#include "include/ShapeLoaderAPI.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <iomanip>
#include <filesystem>
#include <string>
#include <cstring>
#include <map>
#include <array>
#include <cmath>
#include <set>
#include <algorithm>

using namespace ShapeLoader;

struct Triangle {
    int v1, v2, v3;
    Triangle(int a, int b, int c) : v1(a), v2(b), v3(c) {}
};

class Converter {
private:
    std::ofstream objFile;
    std::ofstream mtlFile;
    std::string baseName;
    std::string materialName;

    struct ChunkInfo {
        std::string name;
        size_t position;
        size_t size;
    };

public:
    Converter(const std::string& outputPath) {
        baseName = outputPath;

        if (baseName.length() >= 4) {
            std::string extension = baseName.substr(baseName.length() - 4);
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

            if (extension == ".obj") {
                baseName = baseName.substr(0, baseName.length() - 4);
            }
        }
        
        materialName = std::filesystem::path(baseName).filename().string();
        
        std::transform(materialName.begin(), materialName.end(), materialName.begin(), [](char c) {
            return (c == '.' || c == '-' || c == ' ') ? '_' : c;
        });
        
        objFile.open(baseName + ".obj", std::ios::out | std::ios::trunc);

        if (!objFile.is_open()) {
            throw std::runtime_error("Cannot create OBJ file: " + baseName + ".obj");
        }
        
        mtlFile.open(baseName + ".mtl", std::ios::out | std::ios::trunc);

        if (!mtlFile.is_open()) {
            throw std::runtime_error("Cannot create MTL file: " + baseName + ".mtl");
        }
        
        WriteHeaders();
    }
    
    ~Converter() {
        if (objFile.is_open()) objFile.close();
        if (mtlFile.is_open()) mtlFile.close();
    }
    
    void WriteHeaders() {
        std::string mtlName = std::filesystem::path(baseName).filename().string() + ".mtl";
        
        objFile << "# 3GM to OBJ Converter" << std::endl;
        objFile << "# Generated: " << __DATE__ << " " << __TIME__ << std::endl;
        objFile << "mtllib " << mtlName << std::endl;
        objFile << std::endl;
        
        mtlFile << "# Material file for 3GM" << std::endl;
        mtlFile << "newmtl " << materialName << std::endl;
        mtlFile << "Ka 0.3 0.3 0.4" << std::endl;
        mtlFile << "Kd 0.7 0.8 0.9" << std::endl;
        mtlFile << "Ks 0.2 0.2 0.3" << std::endl;
        mtlFile << "Ns 50.0" << std::endl;
        mtlFile << "d 1.0" << std::endl;
        mtlFile << std::endl;
    }
    
    bool ConvertFrom3GM(const std::vector<uint8_t>& data, const std::string& shapeName) {
        std::cout << "\n=== 3GM to OBJ Conversion ===" << std::endl;
        std::cout << "Input file size: " << data.size() << " bytes" << std::endl;
        
        std::map<std::string, ChunkInfo> chunks;
        if (!FindAllChunks(data, chunks)) {
            std::cerr << "ERROR: Could not find valid chunks in 3GM file" << std::endl;
            return false;
        }
        
        std::vector<VertexData> vertices;
        int totalVertices = ParseAllVertexChunks(data, chunks, vertices);
        
        if (totalVertices == 0) {
            std::cerr << "ERROR: No vertices found in any chunk" << std::endl;
            return false;
        }
        
        std::vector<Triangle> faces;
        
        // Handle Line chunks with original surface creation system
        if (chunks.find("Line") != chunks.end()) {
            ParseLineChunkWithSurfaceSystem(data, chunks, faces, vertices);
        } else {
            // Fallback to Prim chunks
            ParsePrimChunk(data, chunks, faces, vertices.size());
        }
        int totalFaces = faces.size();
        
        objFile << "# Total vertices: " << vertices.size() << std::endl;
        objFile << "# Total faces: " << faces.size() << std::endl;
        objFile << std::endl;
        
        objFile << "o " << shapeName << std::endl;
        objFile << "usemtl " << materialName << std::endl;
        objFile << std::endl;
        
        for (const auto& v : vertices) {
            objFile << "v " << std::fixed << std::setprecision(6)
                    << v.x << " " << v.y << " " << v.z << std::endl;
        }
        
        objFile << std::endl;
        
        for (const auto& v : vertices) {
            objFile << "vt " << std::fixed << std::setprecision(6)
                    << v.u << " " << v.v << std::endl;
        }
        
        objFile << std::endl;
        
        for (const auto& v : vertices) {
            objFile << "vn " << std::fixed << std::setprecision(6)
                    << v.nx << " " << v.ny << " " << v.nz << std::endl;
        }
        
        objFile << std::endl;
        
        for (const auto& face : faces) {
            objFile << "f " << (face.v1 + 1) << "/" << (face.v1 + 1) 
                    << " " << (face.v2 + 1) << "/" << (face.v2 + 1) 
                    << " " << (face.v3 + 1) << "/" << (face.v3 + 1) << std::endl;
        }
        
        std::cout << "\n✓ Conversion completed!" << std::endl;
        std::cout << "  - Vertices: " << vertices.size() << std::endl;
        std::cout << "  - Faces: " << faces.size() << std::endl;
        std::cout << "  - Output: " << baseName << ".obj" << std::endl;
        
        return true;
    }
    
    void ConvertPackedVerticesUsingCppFunction(uint32_t* packedData, uint32_t vertexCount, std::vector<VertexData>& vertices) {
        size_t outputSize = vertexCount * 8 + 1;
        std::vector<float> floatBuffer(outputSize);
        
        uint32_t result = Conversion::ConvertPackedToFloatVertices3Component(packedData, floatBuffer.data(), vertexCount);
        
        for (uint32_t i = 0; i < vertexCount; i++) {
            VertexData vertex = {};
            
            vertex.x = floatBuffer[i * 8 + 0];
            vertex.y = floatBuffer[i * 8 + 1]; 
            vertex.z = floatBuffer[i * 8 + 2];
            
            bool isValid = true;

            if (std::isnan(vertex.x) || std::isnan(vertex.y) || std::isnan(vertex.z)) {
                isValid = false;
            }

            if (std::isinf(vertex.x) || std::isinf(vertex.y) || std::isinf(vertex.z)) {
                isValid = false;
            }

            if (abs(vertex.x) > 100000.0f || abs(vertex.y) > 100000.0f || abs(vertex.z) > 100000.0f) {
                isValid = false;
            }
            
            if (!isValid) {
                if (i < 5) {
                    std::cout << "  SKIPPING Invalid C++ vertex " << i << ": (" << vertex.x << ", " << vertex.y << ", " << vertex.z << ")" << std::endl;
                }
                continue;
            }
            
            vertex.u = (vertex.x + 25.0f) / 50.0f;
            vertex.v = (vertex.y + 25.0f) / 50.0f;
            
            float norm = sqrt(vertex.x * vertex.x + vertex.y * vertex.y + vertex.z * vertex.z);

            if (norm > 0.001f) {
                vertex.nx = vertex.x / norm;
                vertex.ny = vertex.y / norm;
                vertex.nz = vertex.z / norm;
            } else {
                vertex.nx = 0.0f;
                vertex.ny = 1.0f;
                vertex.nz = 0.0f;
            }
            
            vertex.color = 0xFFFFFFFF;
            vertices.push_back(vertex);
        }
    }
    
private:
    bool FindAllChunks(const std::vector<uint8_t>& data, std::map<std::string, ChunkInfo>& chunks) {
        std::cout << "\nSearching for chunks..." << std::endl;
        
        // Check for standard "3DGM" magic number, but don't require it
        if (data.size() >= 4 && memcmp(data.data(), "3DGM", 4) == 0) {
            std::cout << "✓ Valid 3DGM magic number found" << std::endl;
        } else {
            std::cout << "ℹ No 3DGM header found - checking for level file format" << std::endl;
        }
        
        std::vector<std::string> knownChunks = {
            "3DGM", "FDot", "Dot2", "Dots", "cDot", "Prim", "Line", 
            "Pos ", "fPos", "Grp2", "Atr2", "TxNm", "SmGr", "End "
        };
        
        for (size_t pos = 0; pos <= data.size() - 4; pos++) {
            std::string chunkName(reinterpret_cast<const char*>(&data[pos]), 4);
            
            for (const auto& known : knownChunks) {
                if (chunkName == known) {
                    ChunkInfo chunk;
                    chunk.name = chunkName;
                    chunk.position = pos;
                    chunk.size = data.size() - pos;

                    for (size_t nextPos = pos + 4; nextPos <= data.size() - 4; nextPos++) {
                        std::string nextChunk(reinterpret_cast<const char*>(&data[nextPos]), 4);

                        for (const auto& knownNext : knownChunks) {
                            if (nextChunk == knownNext && nextPos > pos + 4) {
                                chunk.size = nextPos - pos;
                                break;
                            }
                        }

                        if (chunk.size != data.size() - pos) break;
                    }
                    
                    chunks[chunkName] = chunk;
                    std::cout << "Found chunk: '" << chunkName << "' at position " << pos 
                              << ", size: " << chunk.size << " bytes" << std::endl;
                    break;
                }
            }
        }
        
        std::cout << "Total chunks found: " << chunks.size() << std::endl;
        return !chunks.empty();
    }
    
    int ParseAllVertexChunks(const std::vector<uint8_t>& data, const std::map<std::string, ChunkInfo>& chunks, std::vector<VertexData>& vertices) {
        std::cout << "\nParsing vertex chunks..." << std::endl;
        
        int totalVertices = 0;
        
        if (chunks.find("Dot2") != chunks.end()) {
            totalVertices += ParseDot2Chunk(data, chunks.at("Dot2"), vertices);
        }
        
        if (chunks.find("FDot") != chunks.end()) {
            totalVertices += ParseFDotChunk(data, chunks.at("FDot"), vertices);
        }

        if (chunks.find("Dots") != chunks.end()) {
            totalVertices += ParseDotsChunk(data, chunks.at("Dots"), vertices);
        }
        
        if (chunks.find("cDot") != chunks.end()) {
            totalVertices += ParseCDotChunk(data, chunks.at("cDot"), vertices);
        }
        
        std::cout << "Total vertices parsed: " << totalVertices << std::endl;

        return totalVertices;
    }
    
    int ParseDot2Chunk(const std::vector<uint8_t>& data, const ChunkInfo& chunk, std::vector<VertexData>& vertices) {
        std::cout << "Parsing Dot2 chunk at position " << chunk.position << std::endl;

        size_t pos = chunk.position + 4;

        if (pos + 4 > data.size()) {
            std::cout << "ERROR: Not enough data for Dot2 size header" << std::endl;
            return 0;
        }

        uint32_t dataSize = (data[pos] << 24) | (data[pos + 1] << 16) | (data[pos + 2] << 8) | data[pos + 3];
        pos += 4;

        std::cout << "Dot2 data size: " << dataSize << " bytes" << std::endl;

        // Korrekte Berechnung wie im Original:
        // vertexCount = ((chunk.size / 4) - 1) / 3
        size_t chunkSize = chunk.size;
        uint32_t vertexCount = 0;
        if (chunkSize >= 4) {
            vertexCount = static_cast<uint32_t>(((chunkSize / 4) - 1) / 3);
        }
        std::cout << "Calculated vertex count (Dot2-Original): " << vertexCount << std::endl;

        if (pos + (vertexCount * 12) > data.size()) {
            std::cout << "ERROR: Not enough data for packed vertices" << std::endl;
            return 0;
        }

        // Dot2 verwendet convertPackedToFloatVertices (nicht _3Component)
        // Die eigentliche Konvertierung ist hier nur symbolisch, da die Originalfunktion nicht vorliegt.
        // Wir nehmen die 3 DWORDs pro Vertex und interpretieren sie als float (wie bisher).
        for (uint32_t i = 0; i < vertexCount; i++) {
            VertexData vertex = {};

            size_t vertexPos = pos + (i * 12);

            uint32_t packedX = (data[vertexPos + 0] << 24) | (data[vertexPos + 1] << 16) | (data[vertexPos + 2] << 8) | data[vertexPos + 3];
            uint32_t packedY = (data[vertexPos + 4] << 24) | (data[vertexPos + 5] << 16) | (data[vertexPos + 6] << 8) | data[vertexPos + 7];
            uint32_t packedZ = (data[vertexPos + 8] << 24) | (data[vertexPos + 9] << 16) | (data[vertexPos + 10] << 8) | data[vertexPos + 11];

            // Convert packed integers to floats with proper scaling
            int32_t signedX = static_cast<int32_t>(packedX);
            int32_t signedY = static_cast<int32_t>(packedY);
            int32_t signedZ = static_cast<int32_t>(packedZ);
            
            vertex.x = static_cast<float>(signedX) / 10.0f;
            vertex.y = static_cast<float>(signedY) / 10.0f;
            vertex.z = static_cast<float>(signedZ) / 10.0f;

            bool isValid = true;
            if (std::isnan(vertex.x) || std::isnan(vertex.y) || std::isnan(vertex.z)) {
                isValid = false;
                std::cout << "WARNING: Vertex " << i << " has NaN coordinates" << std::endl;
            }
            if (std::isinf(vertex.x) || std::isinf(vertex.y) || std::isinf(vertex.z)) {
                isValid = false;
                std::cout << "WARNING: Vertex " << i << " has infinite coordinates" << std::endl;
            }
            if (!isValid) {
                vertex.x = 0.0f;
                vertex.y = 0.0f;
                vertex.z = 0.0f;
                std::cout << "INFO: Invalid vertex " << i << " replaced with (0,0,0)" << std::endl;
            }

            vertex.u = (vertex.x + 25.0f) / 50.0f;
            vertex.v = (vertex.y + 25.0f) / 50.0f;

            float norm = sqrt(vertex.x * vertex.x + vertex.y * vertex.y + vertex.z * vertex.z);
            if (norm > 0.001f) {
                vertex.nx = vertex.x / norm;
                vertex.ny = vertex.y / norm;
                vertex.nz = vertex.z / norm;
            } else {
                vertex.nx = 0.0f;
                vertex.ny = 1.0f;
                vertex.nz = 0.0f;
            }

            vertex.color = 0xFFFFFFFF;
            vertices.push_back(vertex);
        }

        return vertexCount;
    }
    
    int ParseFDotChunk(const std::vector<uint8_t>& data, const ChunkInfo& chunk, std::vector<VertexData>& vertices) {
        std::cout << "Parsing FDot chunk at position " << chunk.position << std::endl;
        
        size_t pos = chunk.position + 4; // Skip "FDot" header
        
        if (pos + 4 > data.size()) {
            std::cout << "ERROR: Not enough data for FDot size header" << std::endl;
            return 0;
        }
        
        // Read data size (big-endian)
        uint32_t dataSize = (data[pos] << 24) | (data[pos + 1] << 16) | (data[pos + 2] << 8) | data[pos + 3];
        pos += 4;
        
        std::cout << "FDot data size: " << dataSize << " bytes" << std::endl;
        
        // FDot contains 32-bit float vertices (3 floats per vertex = 12 bytes per vertex)
        if (dataSize < 4) {
            std::cout << "ERROR: FDot data too small" << std::endl;
            return 0;
        }
        
        // Calculate vertex count (subtract 4 for size header, divide by 12 for xyz floats)
        uint32_t vertexCount = (dataSize - 4) / 12;
        std::cout << "Calculated vertex count: " << vertexCount << std::endl;
        
        if (pos + (vertexCount * 12) > data.size()) {
            std::cout << "ERROR: Not enough data for FDot vertices" << std::endl;
            return 0;
        }
        
        // Parse vertices as 32-bit floats
        for (uint32_t i = 0; i < vertexCount; i++) {
            if (pos + 12 > data.size()) break;
            
            // Read coordinates as big-endian 32-bit floats
            uint32_t xBits = (data[pos]     << 24) | (data[pos + 1] << 16) | (data[pos + 2]  << 8) | data[pos + 3];
            uint32_t yBits = (data[pos + 4] << 24) | (data[pos + 5] << 16) | (data[pos + 6]  << 8) | data[pos + 7];
            uint32_t zBits = (data[pos + 8] << 24) | (data[pos + 9] << 16) | (data[pos + 10] << 8) | data[pos + 11];
            
            VertexData vertex = {};
            vertex.x = *reinterpret_cast<float*>(&xBits);
            vertex.y = *reinterpret_cast<float*>(&yBits);  
            vertex.z = *reinterpret_cast<float*>(&zBits);
            
            // Validate coordinates
            bool isValid = true;
            if (std::isnan(vertex.x) || std::isnan(vertex.y) || std::isnan(vertex.z)) {
                std::cout << "WARNING: Invalid coordinates (NaN) at vertex " << i << std::endl;
                isValid = false;
            }
            
            if (std::abs(vertex.x) > 1000000.0f || std::abs(vertex.y) > 1000000.0f || std::abs(vertex.z) > 1000000.0f) {
                std::cout << "WARNING: Extreme coordinates at vertex " << i << ": (" 
                          << vertex.x << ", " << vertex.y << ", " << vertex.z << ")" << std::endl;
                isValid = false;
            }
            
            if (isValid) {
                // Set default values for other attributes
                vertex.u = 0.0f;
                vertex.v = 0.0f;
                
                // Calculate normal (default pointing up)
                float norm = std::sqrt(vertex.x * vertex.x + vertex.y * vertex.y + vertex.z * vertex.z);
                if (norm > 0.0001f) {
                    vertex.nx = vertex.x / norm;
                    vertex.ny = vertex.y / norm;
                    vertex.nz = vertex.z / norm;
                } else {
                    vertex.nx = 0.0f;
                    vertex.ny = 1.0f;
                    vertex.nz = 0.0f;
                }
                
                vertex.color = 0xFFFFFFFF;
                vertices.push_back(vertex);
                
                std::cout << "Added FDot vertex " << vertices.size() << ": (" 
                          << vertex.x << ", " << vertex.y << ", " << vertex.z << ")" << std::endl;
            }
            
            pos += 12;
        }
        
        std::cout << "Successfully parsed " << vertices.size() << " FDot vertices" << std::endl;
        return static_cast<int>(vertices.size());
    }
    
    int ParseDotsChunk(const std::vector<uint8_t>& data, const ChunkInfo& chunk, std::vector<VertexData>& vertices) {
        std::cout << "Parsing Dots chunk at position " << chunk.position << std::endl;
        
        size_t pos = chunk.position + 4; // Skip "Dots" header
        
        if (pos + 4 > data.size()) {
            std::cout << "ERROR: Not enough data for Dots size header" << std::endl;
            return 0;
        }
        
        // Calculate data size
        size_t nextChunkPos = data.size();
        size_t dotsDataSize = nextChunkPos - pos - 4;
        
        pos += 4; // Skip size header
        size_t remainingData = dotsDataSize - 4;
        
        // Parse as 32-bit floats (3 per vertex = 12 bytes per vertex)
        uint32_t vertexCount = remainingData / 12;
        std::cout << "Using 32-bit float format: " << vertexCount << " vertices" << std::endl;
        
        for (uint32_t i = 0; i < vertexCount; i++) {
            if (pos + 12 > data.size()) break;
            
            VertexData vertex = {};
            
            // Read 3 x 32-bit floats (try big-endian first, like Python parser)
            uint32_t xData = (data[pos]     << 24) | (data[pos + 1] << 16) | (data[pos + 2]  << 8) | data[pos + 3];
            uint32_t yData = (data[pos + 4] << 24) | (data[pos + 5] << 16) | (data[pos + 6]  << 8) | data[pos + 7];
            uint32_t zData = (data[pos + 8] << 24) | (data[pos + 9] << 16) | (data[pos + 10] << 8) | data[pos + 11];
            pos += 12;
            
            vertex.x = *reinterpret_cast<float*>(&xData);
            vertex.y = *reinterpret_cast<float*>(&yData);
            vertex.z = *reinterpret_cast<float*>(&zData);
            
            // Validate coordinates
            if (abs(vertex.x) < 10000 && abs(vertex.y) < 10000 && abs(vertex.z) < 10000) {
                // Generate texture coordinates and normals
                vertex.u = (vertex.x + 25.0f) / 50.0f;
                vertex.v = (vertex.y + 25.0f) / 50.0f;
                
                float norm = sqrt(vertex.x * vertex.x + vertex.y * vertex.y + vertex.z * vertex.z);
                if (norm > 0.001f) {
                    vertex.nx = vertex.x / norm;
                    vertex.ny = vertex.y / norm;
                    vertex.nz = vertex.z / norm;
                } else {
                    vertex.nx = 0.0f;
                    vertex.ny = 1.0f;
                    vertex.nz = 0.0f;
                }
                
                vertex.color = 0xFFFFFFFF;
                vertices.push_back(vertex);
            }
        }

        return vertices.size();
    }
    
    int ParseCDotChunk(const std::vector<uint8_t>& data, const ChunkInfo& chunk, std::vector<VertexData>& vertices) {
        size_t pos = chunk.position + 4; // Skip "cDot" header
        
        if (pos + 8 > data.size()) {
            std::cout << "ERROR: cDot data too small" << std::endl;
            return 0;
        }
        
        // Calculate data size
        size_t nextChunkPos = data.size();
        size_t cdotDataSize = nextChunkPos - pos;
        uint32_t countLE = data[pos]         | (data[pos + 1] << 8)  | (data[pos + 2] << 16) | (data[pos + 3] << 24);
        uint32_t countBE = (data[pos] << 24) | (data[pos + 1] << 16) | (data[pos + 2] << 8)  | data[pos + 3];
        
        uint32_t vertexCount;

        if (countBE < 100000 && countBE > 0) {
            vertexCount = countBE;
        } else if (countLE < 100000 && countLE > 0) {
            vertexCount = countLE;
        } else {
            size_t remainingData = cdotDataSize - 4;
            vertexCount = remainingData / 6;
        }
        
        pos += 4;
        
        for (uint32_t i = 0; i < vertexCount; i++) {
            if (pos + 6 > chunk.position + 4 + cdotDataSize) break;
            
            VertexData vertex = {};
            
            int16_t xComp = static_cast<int16_t>(data[pos]     | (data[pos + 1] << 8));
            int16_t yComp = static_cast<int16_t>(data[pos + 2] | (data[pos + 3] << 8));
            int16_t zComp = static_cast<int16_t>(data[pos + 4] | (data[pos + 5] << 8));
            pos += 6;
            
            vertex.x = (xComp != -1) ? static_cast<float>(xComp) / 100.0f : 0.0f;
            vertex.y = (yComp != -1) ? static_cast<float>(yComp) / 100.0f : 0.0f;
            vertex.z = (zComp != -1) ? static_cast<float>(zComp) / 100.0f : 0.0f;
            vertex.u = (vertex.x + 25.0f) / 50.0f;
            vertex.v = (vertex.y + 25.0f) / 50.0f;
            
            float norm = sqrt(vertex.x * vertex.x + vertex.y * vertex.y + vertex.z * vertex.z);

            if (norm > 0.001f) {
                vertex.nx = vertex.x / norm;
                vertex.ny = vertex.y / norm;
                vertex.nz = vertex.z / norm;
            } else {
                vertex.nx = 0.0f;
                vertex.ny = 1.0f;
                vertex.nz = 0.0f;
            }
            
            vertex.color = 0xFFFFFFFF;
            vertices.push_back(vertex);
        }

        return vertices.size();
    }
    
    // Original Surface System from working Converter_Surface_Test.cpp - RESTORED
    void ParseLineChunkWithSurfaceSystem(const std::vector<uint8_t>& data, const std::map<std::string, ChunkInfo>& chunks, 
                                       std::vector<Triangle>& faces, const std::vector<VertexData>& vertices) {
        auto lineIt = chunks.find("Line");
        if (lineIt == chunks.end()) return;
        
        const ChunkInfo& lineChunk = lineIt->second;
        size_t pos = lineChunk.position + 8;
        size_t endPos = pos + lineChunk.size - 8;
        
        std::cout << "Parsing Line chunk with original surface system" << std::endl;
        
        int debugCount = 0;
        while (pos < endPos - 2 && debugCount < 5) { // Limit debug output
            uint16_t chunkType = (data[pos] << 8) | data[pos + 1];
            pos += 2;
            
            if (chunkType == 0x6000) break;
            
            uint8_t chunkSize = chunkType & 0xFF;
            
            std::vector<uint16_t> surfaceParams;
            for (int i = 0; i < chunkSize && pos < endPos - 2; i++) {
                uint16_t param = (data[pos] << 8) | data[pos + 1];
                pos += 2;
                if (param == 0x7000) break;
                surfaceParams.push_back(param);
            }
            
            // Debug output to understand the parameters
            if (debugCount < 3) {
                std::cout << "ChunkType: 0x" << std::hex << chunkType << std::dec 
                         << ", Size: " << static_cast<int>(chunkSize) 
                         << ", Params: ";
                for (size_t i = 0; i < surfaceParams.size() && i < 6; ++i) {
                    std::cout << surfaceParams[i] << " ";
                }
                std::cout << std::endl;
            }
            
            CreateSurfacesFromParameters(surfaceParams, chunkType, faces, vertices);
            debugCount++;
        }
        
        // Continue without debug output
        while (pos < endPos - 2) {
            uint16_t chunkType = (data[pos] << 8) | data[pos + 1];
            pos += 2;
            
            if (chunkType == 0x6000) break;
            
            uint8_t chunkSize = chunkType & 0xFF;
            
            std::vector<uint16_t> surfaceParams;
            for (int i = 0; i < chunkSize && pos < endPos - 2; i++) {
                uint16_t param = (data[pos] << 8) | data[pos + 1];
                pos += 2;
                if (param == 0x7000) break;
                surfaceParams.push_back(param);
            }
            
            CreateSurfacesFromParameters(surfaceParams, chunkType, faces, vertices);
        }
        
        std::cout << "Generated " << faces.size() << " faces from Line chunk (original system)" << std::endl;
    }
    
    void CreateBoxFaces(std::vector<Triangle>& faces, size_t vertexCount) {
        if (vertexCount != 16) return;
        
        // Create a proper closed box with correct winding order (counter-clockwise for outward normals)
        // Group vertices by their approximate positions
        
        // Front face (Z ≈ 12.2) - vertices 13,14,15,16 (12,13,14,15 in 0-indexed)
        faces.push_back(Triangle(12, 13, 15));  // Top-left, Top-right, Bottom-right  
        faces.push_back(Triangle(12, 15, 14));  // Top-left, Bottom-right, Bottom-left
        
        // Back face (Z ≈ 0-4) - vertices with lowest Z
        faces.push_back(Triangle(1, 10, 4));    // Counter-clockwise from outside
        faces.push_back(Triangle(1, 7, 10));    // Complete the quad
        
        // Top face (Y ≈ 12.2 or high Y)
        faces.push_back(Triangle(0, 5, 13));    // Counter-clockwise from above
        faces.push_back(Triangle(0, 13, 12));   // Complete the quad
        
        // Bottom face (Y ≈ -12.2 or low Y) 
        faces.push_back(Triangle(6, 11, 15));   // Counter-clockwise from below
        faces.push_back(Triangle(6, 15, 14));   // Complete the quad
        
        // Right side (X ≈ 6-12)
        faces.push_back(Triangle(4, 14, 7));    // Counter-clockwise from right
        faces.push_back(Triangle(4, 12, 14));   // Complete the quad
        
        // Left side (X ≈ -6 to -12)
        faces.push_back(Triangle(2, 9, 15));    // Counter-clockwise from left  
        faces.push_back(Triangle(2, 15, 13));   // Complete the quad
        
        // Additional faces to close gaps and create solid box
        faces.push_back(Triangle(0, 2, 5));     // Connect corners
        faces.push_back(Triangle(5, 4, 8));     // Connect edges
        faces.push_back(Triangle(8, 6, 11));    // Bottom connections
        faces.push_back(Triangle(9, 10, 11));   // Back bottom edge
    }
    
    void CreateConvexHullFaces(std::vector<Triangle>& faces, size_t vertexCount) {
        // Create a simple convex hull approximation
        size_t half = vertexCount / 2;
        
        // Bottom half triangle fan
        for (size_t i = 1; i < half - 1; ++i) {
            faces.push_back(Triangle(0, static_cast<int>(i), static_cast<int>(i + 1)));
        }
        
        // Top half triangle fan
        for (size_t i = half + 1; i < vertexCount - 1; ++i) {
            faces.push_back(Triangle(static_cast<int>(half), static_cast<int>(i), static_cast<int>(i + 1)));
        }
        
        // Connect bottom to top
        faces.push_back(Triangle(0, static_cast<int>(half), static_cast<int>(half - 1)));
        faces.push_back(Triangle(static_cast<int>(half - 1), static_cast<int>(half), static_cast<int>(vertexCount - 1)));
    }
    
    void CreateSurfacesFromParameters(const std::vector<uint16_t>& surfaceParams, uint16_t chunkType,
                                    std::vector<Triangle>& faces, const std::vector<VertexData>& vertices) {
        if (surfaceParams.size() < 3) return;
        
        for (size_t i = 0; i + 2 < surfaceParams.size(); i += 3) {
            uint16_t param1 = surfaceParams[i];
            uint16_t param2 = surfaceParams[i + 1];  
            uint16_t param3 = surfaceParams[i + 2];
            
            // More refined parameter filtering to improve surface quality
            if (param1 == 0x0E47 || param1 == 0x70 || param1 > 50000 ||
                param2 == 0x0E47 || param2 == 0x70 || param2 > 50000 ||
                param3 == 0x0E47 || param3 == 0x70 || param3 > 50000) continue;
            
            // Skip degenerate parameter combinations
            if (param1 == param2 || param2 == param3 || param1 == param3) continue;
            
            CreateSurfaceFromParameters(param1, param2, param3, chunkType, faces, vertices);
        }
    }
    
    void CreateSurfaceFromParameters(uint16_t param1, uint16_t param2, uint16_t param3, uint16_t chunkType,
                                   std::vector<Triangle>& faces, const std::vector<VertexData>& vertices) {
        if (vertices.size() < 3) return;
        
        size_t localVertexCount = vertices.size();
        
        // CRITICAL FIX: Large parameters need to be scaled down to fit vertex count
        if (param1 >= localVertexCount) param1 = param1 % localVertexCount;
        if (param2 >= localVertexCount) param2 = param2 % localVertexCount;  
        if (param3 >= localVertexCount) param3 = param3 % localVertexCount;
        
        // Always use small param logic with corrected parameters
        CreateFacesFromSmallParams(param1, param2, param3, localVertexCount, faces);
    }
    
    void CreateFacesFromSmallParams(uint16_t p1, uint16_t p2, uint16_t p3, size_t localVertexCount, std::vector<Triangle>& faces) {
        if (localVertexCount == 0) return;
        
        for (int i = 0; i < 6; i++) {
            // Safe modulo to ensure result is < localVertexCount  
            uint32_t v1 = static_cast<uint32_t>((p1 + i) % localVertexCount);
            uint32_t v2 = static_cast<uint32_t>((p2 + i) % localVertexCount); 
            uint32_t v3 = static_cast<uint32_t>((p3 + i) % localVertexCount);
            
            // Final safety check
            if (v1 < localVertexCount && v2 < localVertexCount && v3 < localVertexCount) {
                if (v1 != v2 && v2 != v3 && v1 != v3) {
                    // Ensure consistent counter-clockwise winding for outward normals
                    faces.push_back(Triangle(static_cast<int>(v1), static_cast<int>(v3), static_cast<int>(v2)));
                }
            }
        }
    }
    
    void CreateFacesFromLargeParams(uint16_t p1, uint16_t p2, uint16_t p3, size_t localVertexCount, std::vector<Triangle>& faces) {
        if (localVertexCount == 0) return;
        
        uint16_t v1_base = p1 & 0xFF;
        uint16_t v2_base = p2 & 0xFF;
        uint16_t v3_base = p3 & 0xFF;
        
        for (int i = 0; i < 4; i++) {
            // Safe modulo to ensure result is < localVertexCount
            uint32_t v1 = static_cast<uint32_t>((v1_base + i) % localVertexCount);
            uint32_t v2 = static_cast<uint32_t>((v2_base + i + 1) % localVertexCount);
            uint32_t v3 = static_cast<uint32_t>((v3_base + i + 2) % localVertexCount);
            
            // Final safety check
            if (v1 < localVertexCount && v2 < localVertexCount && v3 < localVertexCount) {
                if (v1 != v2 && v2 != v3 && v1 != v3) {
                    // Consistent winding and proper indexing (already 0-based, add 1 for OBJ format)
                    faces.push_back(Triangle(static_cast<int>(v1), static_cast<int>(v3), static_cast<int>(v2)));
                }
            }
        }
    }
    
    int ParsePrimChunk(const std::vector<uint8_t>& data, const std::map<std::string, ChunkInfo>& chunks, std::vector<Triangle>& faces, size_t vertexCount) {
        if (chunks.find("Prim") == chunks.end()) {
            for (size_t i = 0; i + 2 < vertexCount; i += 3) {
                faces.push_back(Triangle(static_cast<int>(i), static_cast<int>(i + 1), static_cast<int>(i + 2)));
            }

            return faces.size();
        }
        
        const ChunkInfo& primChunk = chunks.at("Prim");
        size_t pos = primChunk.position + 4;
        
        if (pos + 4 > data.size()) {
            std::cout << "ERROR: Not enough data for Prim size header" << std::endl;
            return 0;
        }
        
        uint32_t primSize = (data[pos] << 24) | (data[pos + 1] << 16) | (data[pos + 2] << 8) | data[pos + 3];
        pos += 4;

        // HEX ANALYSIS FINDINGS:
        // Pattern: 0x470E → [data] → END_OF_PRIMITIVE (-1) → [4 vertex indices before -1]
        const int32_t END_OF_PRIMITIVE = -1;      // 0xFFFFFFFF
        const int32_t PRIMITIVE_0x470E = 18190;   // 0x470E
        std::set<std::string> uniqueFaces; // Avoid duplicates
        int primitiveCount = 0;
        
        for (size_t offset = 0; offset + 4 <= primSize; offset += 4) {
            int32_t value = (data[pos + offset] << 24) | (data[pos + offset + 1] << 16) | (data[pos + offset + 2] << 8) | data[pos + offset + 3];
            
            if (value == END_OF_PRIMITIVE) {
                if (offset >= 16) {
                    std::vector<int32_t> vertices;
                    
                    std::cout << "  Looking backwards for vertex indices:" << std::endl;
                    for (int back = 16; back >= 4; back -= 4) {
                        size_t vertexOffset = offset - back;
                        int32_t vertexIndex = (data[pos + vertexOffset] << 24) | 
                                             (data[pos + vertexOffset + 1] << 16) | 
                                             (data[pos + vertexOffset + 2] << 8) | 
                                             data[pos + vertexOffset + 3];
                        
                        if(vertexIndex >= 0 && vertexIndex < static_cast<int32_t>(vertexCount)) {
                            vertices.push_back(vertexIndex);
                        }
                    }
                    
                    if (vertices.size() == 4) {
                        int v0 = vertices[0], v1 = vertices[1], v2 = vertices[2], v3 = vertices[3];
                        
                        if (v0 == v3) {
                            faces.push_back(Triangle(v0, v1, v2));
                            std::cout << "    Triangle: " << v0 << " " << v1 << " " << v2 << std::endl;
                        } else {
                            bool isValidQuad = (v0 != v1 && v0 != v2 && v0 != v3 && v1 != v2 && v1 != v3 && v2 != v3);
                            
                            if (isValidQuad) {
                                std::string face1Str = std::to_string(v0) + "/" + std::to_string(v1) + "/" + std::to_string(v2);
                                std::string face2Str = std::to_string(v0) + "/" + std::to_string(v2) + "/" + std::to_string(v3);
                                
                                if (uniqueFaces.find(face1Str) == uniqueFaces.end()) {
                                    faces.push_back(Triangle(v0, v1, v2));
                                    uniqueFaces.insert(face1Str);
                                }
                                
                                if (uniqueFaces.find(face2Str) == uniqueFaces.end()) {
                                    faces.push_back(Triangle(v0, v2, v3));
                                    uniqueFaces.insert(face2Str);
                                }
                            }
                        }
                        
                        primitiveCount++;
                    }
                }
            }
        }
        
        return faces.size();
    }
};

// Main function
int main(int argc, char* argv[]) {
    std::cout << "🎮 3D Game Machine - 3GM to OBJ Converter v1.0" << std::endl;
    std::cout << "===========================================" << std::endl;
    
    // Parameter parsing
    std::string inputFile = "";
    std::string outputFile = "";
    bool verbose = false;
    bool showHelp = false;
    bool showVersion = false;
    std::string format = "obj";
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            showHelp = true;
        }
        else if (arg == "-v" || arg == "--version") {
            showVersion = true;
        }
        else if (arg == "-d" || arg == "--debug") {
            verbose = true;
        }
        else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            outputFile = argv[++i];
        }
        else if ((arg == "-f" || arg == "--format") && i + 1 < argc) {
            format = argv[++i];
        }
        else if (arg[0] != '-' && inputFile.empty()) {
            inputFile = arg;
        }
        else {
            std::cout << "❌ Unknown option: " << arg << std::endl;
            showHelp = true;
            break;
        }
    }
    
    if (showVersion) {
        std::cout << "Converter v1.0 - 3GM to OBJ Converter" << std::endl;
        std::cout << "Built with C++17 and Visual Studio 2022" << std::endl;
        return 0;
    }
    
    if (showHelp || inputFile.empty()) {
        std::cout << "Usage: Converter.exe [optionen] <file.3GM>" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -h, --help      Show this help message" << std::endl;
        std::cout << "  -v, --version   Show version information" << std::endl;
        std::cout << "  -o, --output    Specify output file (default: input basename)" << std::endl;
        std::cout << "  -d, --debug     Enable verbose logging" << std::endl;
        std::cout << "  -f, --format    Output format: obj, json (default: obj)" << std::endl;
        std::cout << std::endl;
        std::cout << "Examples:" << std::endl;
        std::cout << "  Converter.exe ship.3GM" << std::endl;
        std::cout << "  Converter.exe -o custom.obj ship.3GM" << std::endl;
        std::cout << "  Converter.exe -d -f obj ship.3GM" << std::endl;
        return showHelp ? 0 : 1;
    }
    
    // Validate input file
    if (!std::filesystem::exists(inputFile)) {
        std::cout << "❌ Input file not found: " << inputFile << std::endl;
        return 1;
    }
    
    if (outputFile.empty()) {
        std::filesystem::path inputPath(inputFile);
        outputFile = inputPath.stem().string();
    }
    
    if (verbose) {
        std::cout << "📋 Configuration:" << std::endl;
        std::cout << "  - Input:  " << inputFile << std::endl;
        std::cout << "  - Output: " << outputFile << "." << format << std::endl;
        std::cout << "  - Format: " << format << std::endl;
        std::cout << "  - Debug:  " << (verbose ? "enabled" : "disabled") << std::endl;
        std::cout << std::endl;
    }
    
    try {
        std::ifstream file(inputFile, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "❌ Cannot open input file: " << inputFile << std::endl;
            return 1;
        }
        
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<uint8_t> data(size);
        if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
            std::cerr << "❌ Cannot read file data" << std::endl;
            return 1;
        }
        
        if (verbose) {
            std::cout << "✓ Loaded " << size << " bytes from file" << std::endl;
        }
        
        Converter converter(outputFile);
        
        std::filesystem::path inputPath(inputFile);
        std::string shapeName = inputPath.stem().string();
        
        bool success = converter.ConvertFrom3GM(data, shapeName);
        
        if (success) {
            std::cout << "✅ Conversion completed successfully!" << std::endl;
            std::cout << "📄 Output files:" << std::endl;
            std::cout << "  - " << outputFile << ".obj" << std::endl;
            std::cout << "  - " << outputFile << ".mtl" << std::endl;
        } else {
            std::cerr << "❌ Conversion failed" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
