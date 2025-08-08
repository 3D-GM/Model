#include "../include/3GMParser.h"
#include "../include/OBJExporter.h"
#include "../include/ErrorHandler.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>

using namespace ShapeLoader;

struct ProgramOptions {
    std::string inputFile;
    std::string outputPath;
    bool debugMode = false;
    bool verbose = false;
    bool includeNormals = true;
    bool includeTextureCoords = true;
    bool includeVertexColors = false;
    bool generateMTL = true;
    bool flipTextureY = true;
    float scale = 1.0f;
    bool showHelp = false;
    bool showVersion = false;
};

void ShowVersion() {
    std::cout << "3GM2OBJ Converter v2.0 - RFC Validated Parser" << std::endl;
    std::cout << "Built with RFC-validated 3GM format support" << std::endl;
    std::cout << "Supports: All chunk types, mixed endianness, animation data" << std::endl;
    std::cout << std::endl;
}

void ShowHelp() {
    ShowVersion();
    std::cout << "Usage: 3GM2OBJ [options] input.3gm [output_path]" << std::endl;
    std::cout << std::endl;
    std::cout << "Arguments:" << std::endl;
    std::cout << "  input.3gm          Input 3GM file to convert" << std::endl;
    std::cout << "  output_path        Output path (without extension, default: same as input)" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help         Show this help message" << std::endl;
    std::cout << "  -v, --version      Show version information" << std::endl;
    std::cout << "  -d, --debug        Enable debug mode with detailed chunk analysis" << std::endl;
    std::cout << "  --verbose          Enable verbose output" << std::endl;
    std::cout << "  -o PATH            Specify output path" << std::endl;
    std::cout << std::endl;
    std::cout << "Export Options:" << std::endl;
    std::cout << "  --no-normals       Don't export vertex normals" << std::endl;
    std::cout << "  --no-texcoords     Don't export texture coordinates" << std::endl;
    std::cout << "  --vertex-colors    Include vertex colors in output" << std::endl;
    std::cout << "  --no-mtl           Don't generate MTL material file" << std::endl;
    std::cout << "  --no-flip-y        Don't flip texture Y coordinates" << std::endl;
    std::cout << "  --scale FACTOR     Scale all vertices by factor (default: 1.0)" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  3GM2OBJ ship.3gm                    # Convert to ship.obj/ship.mtl" << std::endl;
    std::cout << "  3GM2OBJ -o models/ship ship.3gm     # Convert to models/ship.obj" << std::endl;
    std::cout << "  3GM2OBJ -d --verbose ship.3gm       # Debug mode with detailed output" << std::endl;
    std::cout << "  3GM2OBJ --scale 0.1 ship.3gm        # Scale down by 10x" << std::endl;
    std::cout << std::endl;
}

bool ParseArguments(int argc, char* argv[], ProgramOptions& options) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            options.showHelp = true;
            return true;
        }
        else if (arg == "-v" || arg == "--version") {
            options.showVersion = true;
            return true;
        }
        else if (arg == "-d" || arg == "--debug") {
            options.debugMode = true;
        }
        else if (arg == "--verbose") {
            options.verbose = true;
        }
        else if (arg == "-o" && i + 1 < argc) {
            options.outputPath = argv[++i];
        }
        else if (arg == "--no-normals") {
            options.includeNormals = false;
        }
        else if (arg == "--no-texcoords") {
            options.includeTextureCoords = false;
        }
        else if (arg == "--vertex-colors") {
            options.includeVertexColors = true;
        }
        else if (arg == "--no-mtl") {
            options.generateMTL = false;
        }
        else if (arg == "--no-flip-y") {
            options.flipTextureY = false;
        }
        else if (arg == "--scale" && i + 1 < argc) {
            try {
                options.scale = std::stof(argv[++i]);
                if (options.scale <= 0.0f) {
                    std::cerr << "Error: Scale factor must be positive" << std::endl;
                    return false;
                }
            } catch (const std::exception&) {
                std::cerr << "Error: Invalid scale factor" << std::endl;
                return false;
            }
        }
        else if (arg[0] == '-') {
            std::cerr << "Error: Unknown option: " << arg << std::endl;
            return false;
        }
        else {
            // Positional arguments
            if (options.inputFile.empty()) {
                options.inputFile = arg;
            }
            else if (options.outputPath.empty()) {
                options.outputPath = arg;
            }
            else {
                std::cerr << "Error: Too many arguments" << std::endl;
                return false;
            }
        }
    }
    
    return true;
}

bool ValidateOptions(const ProgramOptions& options) {
    if (options.showHelp || options.showVersion) {
        return true;
    }
    
    if (options.inputFile.empty()) {
        std::cerr << "Error: No input file specified" << std::endl;
        return false;
    }
    
    if (!std::filesystem::exists(options.inputFile)) {
        std::cerr << "Error: Input file does not exist: " << options.inputFile << std::endl;
        return false;
    }
    
    return true;
}

std::string GenerateOutputPath(const std::string& inputFile, const std::string& outputPath) {
    if (!outputPath.empty()) {
        return outputPath;
    }
    
    std::filesystem::path p(inputFile);
    return p.stem().string();
}

void PrintSummary(const ShapeData& shapeData, double parseTime, double exportTime) {
    std::cout << std::endl;
    std::cout << "📊 Conversion Summary:" << std::endl;
    std::cout << "   Vertices: " << shapeData.vertexCount << std::endl;
    std::cout << "   Primitives: " << shapeData.primitiveCount << std::endl;
    std::cout << "   Surfaces: " << shapeData.surfaceCount << std::endl;
    
    if (shapeData.hasAnimation) {
        std::cout << "   Animation: Yes (keyframes: " << shapeData.animationFrameCount << ")" << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "⏱️  Performance:" << std::endl;
    std::cout << "   Parse Time: " << std::fixed << std::setprecision(2) << parseTime << "ms" << std::endl;
    std::cout << "   Export Time: " << std::fixed << std::setprecision(2) << exportTime << "ms" << std::endl;
    std::cout << "   Total Time: " << std::fixed << std::setprecision(2) << (parseTime + exportTime) << "ms" << std::endl;
}

int main(int argc, char* argv[]) {
    ProgramOptions options;
    
    if (!ParseArguments(argc, argv, options)) {
        std::cerr << "Use --help for usage information" << std::endl;
        return 1;
    }
    
    if (options.showHelp) {
        ShowHelp();
        return 0;
    }
    
    if (options.showVersion) {
        ShowVersion();
        return 0;
    }
    
    if (!ValidateOptions(options)) {
        return 1;
    }
    
    // Initialize error handler
    ErrorHandler::SetVerbose(options.verbose);
    
    if (options.verbose) {
        ShowVersion();
        std::cout << "🔄 Processing: " << options.inputFile << std::endl;
        std::cout << std::endl;
    }
    
    // Read input file
    std::ifstream file(options.inputFile, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open input file: " << options.inputFile << std::endl;
        return 1;
    }
    
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(fileSize);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    file.close();
    
    if (options.verbose) {
        std::cout << "📁 File Size: " << fileSize << " bytes" << std::endl;
    }
    
    // Parse 3GM file
    Parser3GM parser;
    
    auto parseStart = std::chrono::high_resolution_clock::now();
    bool parseSuccess = parser.ParseBuffer(data.data(), fileSize, std::filesystem::path(options.inputFile).filename().string());
    auto parseEnd = std::chrono::high_resolution_clock::now();
    
    double parseTime = std::chrono::duration<double, std::milli>(parseEnd - parseStart).count();
    
    if (!parseSuccess) {
        std::cerr << "Error: Failed to parse 3GM file" << std::endl;
        if (options.debugMode) {
            std::cerr << "Enable verbose mode for detailed error information" << std::endl;
        }
        return 1;
    }
    
    const ShapeData& shapeData = parser.GetShapeData();
    
    if (options.verbose || options.debugMode) {
        std::cout << "✅ Parse successful!" << std::endl;
        if (options.debugMode) {
            parser.PrintDebugInfo();
        }
    }
    
    // Generate output path
    std::string outputPath = GenerateOutputPath(options.inputFile, options.outputPath);
    
    // Export to OBJ
    OBJExporter exporter;
    OBJExporter::ExportOptions exportOptions;
    exportOptions.includeNormals = options.includeNormals;
    exportOptions.includeTextureCoords = options.includeTextureCoords;
    exportOptions.includeVertexColors = options.includeVertexColors;
    exportOptions.generateMTL = options.generateMTL;
    exportOptions.flipTextureY = options.flipTextureY;
    exportOptions.scale = options.scale;
    
    auto exportStart = std::chrono::high_resolution_clock::now();
    bool exportSuccess = exporter.ExportToOBJ(shapeData, outputPath, exportOptions);
    auto exportEnd = std::chrono::high_resolution_clock::now();
    
    double exportTime = std::chrono::duration<double, std::milli>(exportEnd - exportStart).count();
    
    if (!exportSuccess) {
        std::cerr << "Error: Failed to export OBJ file" << std::endl;
        return 1;
    }
    
    if (options.verbose) {
        PrintSummary(shapeData, parseTime, exportTime);
    }
    
    std::cout << "✅ Conversion completed successfully!" << std::endl;
    
    return 0;
}