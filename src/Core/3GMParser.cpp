#include "3GMParser.h"
#include "ErrorHandler.h"
#include "GlobalVariables.h"
#include <fstream>
#include <iostream>

Parser3GM::Parser3GM() 
    : debugMode_(false), processedChunkCount_(0) {
    
    // Initialize global systems
    GlobalVariables::InitializeGlobals();
}

Parser3GM::~Parser3GM() {
    Reset();
}

void Parser3GM::RegisterChunkProcessor(ChunkType type, std::unique_ptr<ChunkProcessor> processor) {
    if (processor) {
        chunkProcessors_[type] = std::move(processor);
        
        if (debugMode_) {
            std::cout << "Registered processor for chunk type: " 
                      << ChunkTypeToString(type) << std::endl;
        }
    }
}

void Parser3GM::RegisterDefaultProcessors() {
    // TODO: Register all chunk processors when they're implemented
    // RegisterChunkProcessor(ChunkType::Dot2, std::make_unique<Dot2ChunkProcessor>());
    // RegisterChunkProcessor(ChunkType::Line, std::make_unique<LineChunkProcessor>());
    // RegisterChunkProcessor(ChunkType::soPF, std::make_unique<soPFChunkProcessor>());
    // etc.
    
    if (debugMode_) {
        std::cout << "Registered " << chunkProcessors_.size() << " default chunk processors" << std::endl;
    }
}

bool Parser3GM::ParseFile(const std::string& filename) {
    Reset();
    filename_ = filename;
    
    if (debugMode_) {
        std::cout << "🎮 3GM Parser - Starting file: " << filename << std::endl;
    }
    
    // Load file data
    if (!LoadFileData(filename)) {
        return false;
    }
    
    // Parse from loaded buffer
    return ParseBuffer(fileData_.data(), fileData_.size(), filename);
}

bool Parser3GM::ParseBuffer(const uint8_t* data, size_t size, const std::string& debugName) {
    if (!data || size < 8) {
        ErrorHandler::PostEvent(0x6A, "Invalid buffer data");
        return false;
    }
    
    if (debugMode_) {
        std::cout << "📋 Buffer size: " << size << " bytes" << std::endl;
    }
    
    // Step 1: Detect and process file header
    fileHeader_ = HeaderDetector::DetectHeader(data, size);
    if (!HeaderDetector::ValidateHeader(fileHeader_, data, size)) {
        ErrorHandler::PostEvent(0x6A, "Invalid file header");
        return false;
    }
    
    if (debugMode_) {
        std::cout << "✓ Header detected: " << static_cast<int>(fileHeader_.type) 
                  << " (offset: " << fileHeader_.chunkOffset << ")" << std::endl;
    }
    
    // Step 2: Initialize chunk reader
    chunkReader_ = std::make_unique<ChunkReader>(data, size, fileHeader_.chunkOffset);
    
    // Step 3: Scan all chunks
    if (!chunkReader_->ScanAllChunks()) {
        ErrorHandler::PostEvent(0x6A, "Failed to scan chunks");
        return false;
    }
    
    if (debugMode_) {
        chunkReader_->PrintChunkSummary();
    }
    
    // Step 4: Validate chunk structure
    if (!chunkReader_->ValidateChunkStructure()) {
        ErrorHandler::PostEvent(0x6A, "Invalid chunk structure");
        return false;
    }
    
    // Step 5: Process all chunks
    if (!ProcessAllChunks()) {
        return false;
    }
    
    // Step 6: Validate final parsed data
    if (!ValidateParsedData()) {
        ErrorHandler::PostEvent(0x6A, "Parsed data validation failed");
        return false;
    }
    
    if (debugMode_) {
        PrintParsingSummary();
    }
    
    return true;
}

bool Parser3GM::LoadFileData(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        ErrorHandler::PostEvent(0x6A, "Could not open file: " + filename);
        return false;
    }
    
    size_t fileSize = file.tellg();
    if (fileSize == 0) {
        ErrorHandler::PostEvent(0x6A, "Empty file");
        return false;
    }
    
    fileData_.resize(fileSize);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(fileData_.data()), fileSize);
    
    if (debugMode_) {
        std::cout << "✓ Loaded " << fileSize << " bytes from file" << std::endl;
    }
    
    return true;
}

bool Parser3GM::ProcessAllChunks() {
    if (!chunkReader_) {
        return false;
    }
    
    const auto& chunks = chunkReader_->GetDiscoveredChunks();
    processedChunkCount_ = 0;
    
    for (const auto& header : chunks) {
        // Skip End chunk - it's just a terminator
        if (header.IsEndMarker()) {
            continue;
        }
        
        // Get chunk data
        const uint8_t* chunkData = chunkReader_->GetChunkData(header);
        if (!chunkData) {
            ErrorHandler::PostEvent(0x6A, "Could not get chunk data");
            return false;
        }
        
        // Process chunk
        if (!ProcessChunk(header, chunkData)) {
            if (debugMode_) {
                std::cout << "❌ Failed to process chunk: " << header.GetName() << std::endl;
            }
            return false;
        }
        
        processedChunkCount_++;
    }
    
    return true;
}

bool Parser3GM::ProcessChunk(const ChunkHeader& header, const uint8_t* data) {
    auto it = chunkProcessors_.find(header.type);
    if (it == chunkProcessors_.end()) {
        if (debugMode_) {
            std::cout << "⚠️  No processor for chunk type: " << header.GetName() << std::endl;
        }
        return true;  // Skip unknown chunks gracefully
    }
    
    if (debugMode_) {
        std::cout << "🔄 Processing " << header.GetName() 
                  << " chunk (" << header.size << " bytes)" << std::endl;
    }
    
    return it->second->ProcessChunk(header, data, parsedShape_);
}

const std::vector<ChunkHeader>& Parser3GM::GetDiscoveredChunks() const {
    static const std::vector<ChunkHeader> empty;
    return chunkReader_ ? chunkReader_->GetDiscoveredChunks() : empty;
}

void Parser3GM::Reset() {
    chunkProcessors_.clear();
    fileData_.clear();
    filename_.clear();
    chunkReader_.reset();
    parsedShape_.Reset();
    processedChunkCount_ = 0;
    ErrorHandler::ClearError();
}

bool Parser3GM::ValidateParsedData() const {
    return parsedShape_.IsValid();
}

void Parser3GM::PrintParsingSummary() const {
    std::cout << "\\n✅ Parsing completed successfully!" << std::endl;
    std::cout << "  - Processed chunks: " << processedChunkCount_ << std::endl;
    std::cout << "  - Vertices: " << parsedShape_.GetVertexCount() << std::endl;
    std::cout << "  - Primitives: " << parsedShape_.GetPrimitiveCount() << std::endl;
    std::cout << "  - Surfaces: " << parsedShape_.GetSurfaceCount() << std::endl;
    std::cout << "  - Animated: " << (parsedShape_.IsAnimated() ? "Yes" : "No") << std::endl;
    std::cout << "==========================================\\n" << std::endl;
}