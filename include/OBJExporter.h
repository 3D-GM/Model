#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include "ShapeData.h"

namespace ShapeLoader {

class OBJExporter {
public:
    struct ExportOptions {
        bool includeNormals = true;
        bool includeTextureCoords = true;
        bool includeVertexColors = false;
        bool generateMTL = true;
        bool flipTextureY = true;
        float scale = 1.0f;
    };

    OBJExporter();
    ~OBJExporter();

    bool ExportToOBJ(const ShapeData& shapeData, const std::string& outputPath, const ExportOptions& options = ExportOptions());

private:
    struct MaterialInfo {
        std::string name;
        int textureID = -1;
        float ambient[3] = {0.2f, 0.2f, 0.2f};
        float diffuse[3] = {0.8f, 0.8f, 0.8f};
        float specular[3] = {1.0f, 1.0f, 1.0f};
        float shininess = 32.0f;
        float transparency = 1.0f;
    };

    bool WriteOBJFile(const ShapeData& shapeData, const std::string& objPath, const ExportOptions& options);
    bool WriteMTLFile(const std::vector<MaterialInfo>& materials, const std::string& mtlPath);
    
    std::vector<MaterialInfo> ExtractMaterials(const ShapeData& shapeData);
    std::string GenerateMaterialName(int materialID, int textureID);
    std::string GetBaseName(const std::string& path);
    
    void WriteVertex(std::ofstream& file, const float* vertex, const ExportOptions& options);
    void WriteNormal(std::ofstream& file, const float* normal);
    void WriteTextureCoord(std::ofstream& file, const float* texCoord, const ExportOptions& options);
    void WriteFace(std::ofstream& file, const std::vector<int>& indices, bool hasNormals, bool hasTexCoords);

    std::ofstream objFile_;
    std::ofstream mtlFile_;
    std::string baseName_;
    int vertexOffset_;
    int normalOffset_;
    int texCoordOffset_;
};

} // namespace ShapeLoader