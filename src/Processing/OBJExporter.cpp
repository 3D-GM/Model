#include "../../include/OBJExporter.h"
#include "../../include/ErrorHandler.h"
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <sstream>

namespace ShapeLoader {

OBJExporter::OBJExporter() : vertexOffset_(1), normalOffset_(1), texCoordOffset_(1) {
}

OBJExporter::~OBJExporter() {
    if (objFile_.is_open()) objFile_.close();
    if (mtlFile_.is_open()) mtlFile_.close();
}

bool OBJExporter::ExportToOBJ(const ShapeData& shapeData, const std::string& outputPath, const ExportOptions& options) {
    baseName_ = GetBaseName(outputPath);
    
    std::string objPath = baseName_ + ".obj";
    std::string mtlPath = baseName_ + ".mtl";
    
    // Reset counters
    vertexOffset_ = 1;
    normalOffset_ = 1;
    texCoordOffset_ = 1;
    
    if (!WriteOBJFile(shapeData, objPath, options)) {
        ErrorHandler::LogError("OBJExporter", "Failed to write OBJ file: " + objPath);
        return false;
    }
    
    if (options.generateMTL) {
        auto materials = ExtractMaterials(shapeData);
        if (!WriteMTLFile(materials, mtlPath)) {
            ErrorHandler::LogError("OBJExporter", "Failed to write MTL file: " + mtlPath);
            return false;
        }
    }
    
    std::cout << "✅ Exported: " << objPath << std::endl;
    if (options.generateMTL) {
        std::cout << "✅ Materials: " << mtlPath << std::endl;
    }
    
    return true;
}

bool OBJExporter::WriteOBJFile(const ShapeData& shapeData, const std::string& objPath, const ExportOptions& options) {
    objFile_.open(objPath);
    if (!objFile_.is_open()) {
        return false;
    }
    
    // Write header
    objFile_ << "# 3GM to OBJ Converter - RFC Validated Parser" << std::endl;
    objFile_ << "# Generated from Clusterball 3GM file" << std::endl;
    objFile_ << "# Vertex count: " << shapeData.vertexCount << std::endl;
    objFile_ << "# Primitive count: " << shapeData.primitiveCount << std::endl;
    objFile_ << std::endl;
    
    // Reference MTL file if generating materials
    if (options.generateMTL) {
        std::string mtlName = std::filesystem::path(baseName_).filename().string() + ".mtl";
        objFile_ << "mtllib " << mtlName << std::endl << std::endl;
    }
    
    // Write vertices
    if (shapeData.vertexData && shapeData.vertexCount > 0) {
        objFile_ << "# Vertices" << std::endl;
        for (uint32_t i = 0; i < shapeData.vertexCount; ++i) {
            const float* vertex = &shapeData.vertexData[i * shapeData.vertexStride];
            WriteVertex(objFile_, vertex, options);
        }
        objFile_ << std::endl;
    }
    
    // Write normals
    if (options.includeNormals && shapeData.normalData && shapeData.vertexCount > 0) {
        objFile_ << "# Normals" << std::endl;
        for (uint32_t i = 0; i < shapeData.vertexCount; ++i) {
            const float* normal = &shapeData.normalData[i * 3];
            WriteNormal(objFile_, normal);
        }
        objFile_ << std::endl;
    }
    
    // Write texture coordinates
    if (options.includeTextureCoords && shapeData.textureCoordData && shapeData.vertexCount > 0) {
        objFile_ << "# Texture Coordinates" << std::endl;
        for (uint32_t i = 0; i < shapeData.vertexCount; ++i) {
            const float* texCoord = &shapeData.textureCoordData[i * 2];
            WriteTextureCoord(objFile_, texCoord, options);
        }
        objFile_ << std::endl;
    }
    
    // Write faces from primitives
    if (shapeData.primitiveData && shapeData.primitiveCount > 0) {
        objFile_ << "# Faces" << std::endl;
        
        std::string currentMaterial = "";
        
        for (uint32_t i = 0; i < shapeData.primitiveCount; ++i) {
            const PrimitiveData& prim = shapeData.primitiveData[i];
            
            // Switch material if needed
            if (options.generateMTL) {
                std::string materialName = GenerateMaterialName(prim.materialID, prim.textureID);
                if (materialName != currentMaterial) {
                    objFile_ << "usemtl " << materialName << std::endl;
                    currentMaterial = materialName;
                }
            }
            
            // Convert primitive to faces based on type
            std::vector<int> indices;
            
            switch (prim.type) {
                case PRIMITIVE_TRIANGLE:
                    if (prim.indexCount >= 3) {
                        for (uint32_t j = 0; j < prim.indexCount - 2; j += 3) {
                            indices = {
                                prim.indices[j] + 1,
                                prim.indices[j + 1] + 1,
                                prim.indices[j + 2] + 1
                            };
                            WriteFace(objFile_, indices, options.includeNormals, options.includeTextureCoords);
                        }
                    }
                    break;
                    
                case PRIMITIVE_TRIANGLE_STRIP:
                    if (prim.indexCount >= 3) {
                        for (uint32_t j = 0; j < prim.indexCount - 2; ++j) {
                            if (j % 2 == 0) {
                                indices = {
                                    prim.indices[j] + 1,
                                    prim.indices[j + 1] + 1,
                                    prim.indices[j + 2] + 1
                                };
                            } else {
                                indices = {
                                    prim.indices[j + 1] + 1,
                                    prim.indices[j] + 1,
                                    prim.indices[j + 2] + 1
                                };
                            }
                            WriteFace(objFile_, indices, options.includeNormals, options.includeTextureCoords);
                        }
                    }
                    break;
                    
                case PRIMITIVE_QUAD_STRIP:
                    if (prim.indexCount >= 4) {
                        for (uint32_t j = 0; j < prim.indexCount - 3; j += 2) {
                            // First triangle
                            indices = {
                                prim.indices[j] + 1,
                                prim.indices[j + 1] + 1,
                                prim.indices[j + 2] + 1
                            };
                            WriteFace(objFile_, indices, options.includeNormals, options.includeTextureCoords);
                            
                            // Second triangle
                            indices = {
                                prim.indices[j + 1] + 1,
                                prim.indices[j + 3] + 1,
                                prim.indices[j + 2] + 1
                            };
                            WriteFace(objFile_, indices, options.includeNormals, options.includeTextureCoords);
                        }
                    }
                    break;
                    
                default:
                    // For other primitive types, treat as triangle list
                    if (prim.indexCount >= 3) {
                        for (uint32_t j = 0; j < prim.indexCount - 2; j += 3) {
                            indices = {
                                prim.indices[j] + 1,
                                prim.indices[j + 1] + 1,
                                prim.indices[j + 2] + 1
                            };
                            WriteFace(objFile_, indices, options.includeNormals, options.includeTextureCoords);
                        }
                    }
                    break;
            }
        }
    }
    
    objFile_.close();
    return true;
}

bool OBJExporter::WriteMTLFile(const std::vector<MaterialInfo>& materials, const std::string& mtlPath) {
    mtlFile_.open(mtlPath);
    if (!mtlFile_.is_open()) {
        return false;
    }
    
    mtlFile_ << "# 3GM Material File - RFC Validated Parser" << std::endl;
    mtlFile_ << "# Generated from Clusterball 3GM file" << std::endl;
    mtlFile_ << std::endl;
    
    for (const auto& mat : materials) {
        mtlFile_ << "newmtl " << mat.name << std::endl;
        mtlFile_ << "Ka " << std::fixed << std::setprecision(6) 
                 << mat.ambient[0] << " " << mat.ambient[1] << " " << mat.ambient[2] << std::endl;
        mtlFile_ << "Kd " << std::fixed << std::setprecision(6)
                 << mat.diffuse[0] << " " << mat.diffuse[1] << " " << mat.diffuse[2] << std::endl;
        mtlFile_ << "Ks " << std::fixed << std::setprecision(6)
                 << mat.specular[0] << " " << mat.specular[1] << " " << mat.specular[2] << std::endl;
        mtlFile_ << "Ns " << std::fixed << std::setprecision(2) << mat.shininess << std::endl;
        mtlFile_ << "d " << std::fixed << std::setprecision(6) << mat.transparency << std::endl;
        
        if (mat.textureID >= 0) {
            mtlFile_ << "map_Kd texture_" << mat.textureID << ".tga" << std::endl;
        }
        
        mtlFile_ << std::endl;
    }
    
    mtlFile_.close();
    return true;
}

std::vector<OBJExporter::MaterialInfo> OBJExporter::ExtractMaterials(const ShapeData& shapeData) {
    std::vector<MaterialInfo> materials;
    std::set<std::pair<int, int>> uniqueMaterials;
    
    // Collect unique material/texture combinations
    if (shapeData.primitiveData) {
        for (uint32_t i = 0; i < shapeData.primitiveCount; ++i) {
            const PrimitiveData& prim = shapeData.primitiveData[i];
            uniqueMaterials.insert({prim.materialID, prim.textureID});
        }
    }
    
    // Create materials
    for (const auto& matPair : uniqueMaterials) {
        MaterialInfo mat;
        mat.name = GenerateMaterialName(matPair.first, matPair.second);
        mat.textureID = matPair.second;
        
        // Generate colors based on material ID
        float hue = (matPair.first * 137.5f) / 360.0f;  // Golden angle distribution
        hue = hue - std::floor(hue);  // Keep fractional part
        
        // Simple HSV to RGB conversion for variety
        float saturation = 0.7f;
        float value = 0.8f;
        
        int h_i = static_cast<int>(hue * 6);
        float f = hue * 6 - h_i;
        float p = value * (1 - saturation);
        float q = value * (1 - f * saturation);
        float t = value * (1 - (1 - f) * saturation);
        
        switch (h_i % 6) {
            case 0: mat.diffuse[0] = value; mat.diffuse[1] = t; mat.diffuse[2] = p; break;
            case 1: mat.diffuse[0] = q; mat.diffuse[1] = value; mat.diffuse[2] = p; break;
            case 2: mat.diffuse[0] = p; mat.diffuse[1] = value; mat.diffuse[2] = t; break;
            case 3: mat.diffuse[0] = p; mat.diffuse[1] = q; mat.diffuse[2] = value; break;
            case 4: mat.diffuse[0] = t; mat.diffuse[1] = p; mat.diffuse[2] = value; break;
            case 5: mat.diffuse[0] = value; mat.diffuse[1] = p; mat.diffuse[2] = q; break;
        }
        
        materials.push_back(mat);
    }
    
    return materials;
}

std::string OBJExporter::GenerateMaterialName(int materialID, int textureID) {
    std::stringstream ss;
    ss << "material_" << materialID;
    if (textureID >= 0) {
        ss << "_tex_" << textureID;
    }
    return ss.str();
}

std::string OBJExporter::GetBaseName(const std::string& path) {
    std::filesystem::path p(path);
    
    // Remove .obj extension if present
    if (p.extension() == ".obj") {
        p = p.stem();
    }
    
    return p.string();
}

void OBJExporter::WriteVertex(std::ofstream& file, const float* vertex, const ExportOptions& options) {
    file << "v " << std::fixed << std::setprecision(6)
         << vertex[0] * options.scale << " "
         << vertex[1] * options.scale << " "
         << vertex[2] * options.scale;
    
    if (options.includeVertexColors && vertex[3] != 0.0f) {
        file << " " << vertex[3] << " " << vertex[4] << " " << vertex[5];
    }
    
    file << std::endl;
}

void OBJExporter::WriteNormal(std::ofstream& file, const float* normal) {
    file << "vn " << std::fixed << std::setprecision(6)
         << normal[0] << " " << normal[1] << " " << normal[2] << std::endl;
}

void OBJExporter::WriteTextureCoord(std::ofstream& file, const float* texCoord, const ExportOptions& options) {
    float u = texCoord[0];
    float v = options.flipTextureY ? (1.0f - texCoord[1]) : texCoord[1];
    
    file << "vt " << std::fixed << std::setprecision(6)
         << u << " " << v << std::endl;
}

void OBJExporter::WriteFace(std::ofstream& file, const std::vector<int>& indices, bool hasNormals, bool hasTexCoords) {
    file << "f";
    for (int idx : indices) {
        file << " " << idx;
        if (hasTexCoords || hasNormals) {
            file << "/";
            if (hasTexCoords) file << idx;
            if (hasNormals) {
                file << "/" << idx;
            }
        }
    }
    file << std::endl;
}

} // namespace ShapeLoader