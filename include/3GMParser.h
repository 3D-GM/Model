#pragma once

#include "ShapeData.h"
#include "ChunkProcessor.h"
#include "HeaderDetector.h"
#include "ChunkReader.h"
#include <string>
#include <memory>
#include <map>

/**
 * Main 3GM file parser controller
 * Coordinates header detection, chunk reading, and processing
 */
class Parser3GM {
private:
    // Chunk processors registry
    std::map<ChunkType, std::unique_ptr<ChunkProcessor>> chunkProcessors_;
    
    // File data
    std::vector<uint8_t> fileData_;
    std::string filename_;
    
    // Parsing state
    FileHeader fileHeader_;
    std::unique_ptr<ChunkReader> chunkReader_;
    ShapeData parsedShape_;
    
    // Debug and statistics
    bool debugMode_;
    size_t processedChunkCount_;
    
public:
    Parser3GM();
    ~Parser3GM();
    
    /**
     * Register chunk processor for specific chunk type
     */
    void RegisterChunkProcessor(ChunkType type, std::unique_ptr<ChunkProcessor> processor);
    
    /**
     * Register all default chunk processors
     */
    void RegisterDefaultProcessors();
    
    /**
     * Parse 3GM file from disk
     * @param filename Path to 3GM file
     * @return true if parsing succeeded
     */
    bool ParseFile(const std::string& filename);
    
    /**
     * Parse 3GM data from memory buffer
     * @param data File data buffer
     * @param size Buffer size
     * @param debugName Optional name for debugging
     * @return true if parsing succeeded
     */
    bool ParseBuffer(const uint8_t* data, size_t size, const std::string& debugName = "");
    
    /**
     * Get parsed shape data
     */
    const ShapeData& GetShapeData() const { return parsedShape_; }
    const ShapeData& GetParsedShape() const { return parsedShape_; }
    ShapeData& GetParsedShape() { return parsedShape_; }
    
    /**
     * Get file header information
     */
    const FileHeader& GetFileHeader() const { return fileHeader_; }
    
    /**
     * Get discovered chunks information
     */
    const std::vector<ChunkHeader>& GetDiscoveredChunks() const;
    
    /**
     * Enable/disable debug output
     */
    void SetDebugMode(bool enabled) { debugMode_ = enabled; }
    bool IsDebugMode() const { return debugMode_; }
    
    /**
     * Get parsing statistics
     */
    size_t GetProcessedChunkCount() const { return processedChunkCount_; }
    
    /**
     * Reset parser state for new file
     */
    void Reset();
    
    /**
     * Validate parsed data integrity
     */
    bool ValidateParsedData() const;
    
    /**
     * Print comprehensive parsing summary
     */
    void PrintParsingSummary() const;
    
    /**
     * Print debug information (for compatibility)
     */
    void PrintDebugInfo() const { PrintParsingSummary(); }
    
private:
    /**
     * Load file from disk into memory
     */
    bool LoadFileData(const std::string& filename);
    
    /**
     * Detect and validate file header
     */
    bool ProcessFileHeader();
    
    /**
     * Process all chunks in file
     */
    bool ProcessAllChunks();
    
    /**
     * Process individual chunk with appropriate processor
     */
    bool ProcessChunk(const ChunkHeader& header, const uint8_t* data);
};