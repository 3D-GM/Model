#include "ChunkReader.h"
#include "ByteSwap.h"
#include "ErrorHandler.h"
#include <iostream>
#include <iomanip>

ChunkReader::ChunkReader(const uint8_t* data, size_t size, size_t startOffset)
    : fileData_(data), fileSize_(size), currentOffset_(startOffset) {
    
    if (!fileData_) {
        ErrorHandler::PostEvent(0x6A, "Null file data in ChunkReader");
    }
}

bool ChunkReader::ScanAllChunks() {
    if (!fileData_) {
        return false;
    }
    
    discoveredChunks_.clear();
    Reset();
    
    ChunkHeader header;
    while (ReadNextChunkHeader(header)) {
        discoveredChunks_.push_back(header);
        
        // Check for end chunk - RFC validated terminator
        if (header.IsEndMarker()) {
            break;
        }
        
        // Skip to next chunk
        if (!SkipToNextChunk(header)) {
            ErrorHandler::PostEvent(0x6A, "Failed to skip to next chunk");
            break;
        }
    }
    
    return !discoveredChunks_.empty();
}

bool ChunkReader::ReadNextChunkHeader(ChunkHeader& header) {
    // Need at least 8 bytes for chunk header
    if (currentOffset_ + 8 > fileSize_) {
        return false;
    }
    
    // RFC VALIDATED: Chunk header structure
    // ChunkID (4 bytes) + Size (4 bytes), both little-endian
    uint32_t chunkID = ByteSwap::ReadLittleEndian32(fileData_ + currentOffset_);
    uint32_t chunkSize = ByteSwap::ReadLittleEndian32(fileData_ + currentOffset_ + 4);
    
    // Create header
    header = ChunkHeader(chunkID, chunkSize);
    
    // Validate chunk doesn't extend past file
    if (currentOffset_ + header.GetTotalSize() > fileSize_) {
        ErrorHandler::PostEvent(0x6A, "Chunk extends past end of file");
        return false;
    }
    
    return header.IsValid();
}

const uint8_t* ChunkReader::GetChunkData(const ChunkHeader& header) const {
    if (!fileData_ || currentOffset_ + 8 >= fileSize_) {
        return nullptr;
    }
    
    // Data starts after 8-byte header
    return fileData_ + currentOffset_ + 8;
}

bool ChunkReader::SkipToNextChunk(const ChunkHeader& header) {
    // RFC VALIDATED: Total chunk size = 8 bytes header + data size
    size_t totalSize = header.GetTotalSize();
    
    if (currentOffset_ + totalSize > fileSize_) {
        return false;
    }
    
    currentOffset_ += totalSize;
    return true;
}

void ChunkReader::Reset() {
    // Reset to the original start offset (after header)
    size_t startOffset = (fileSize_ >= 8) ? 
        (discoveredChunks_.empty() ? currentOffset_ : currentOffset_ - GetCurrentOffset()) : 0;
    currentOffset_ = startOffset;
}

bool ChunkReader::IsAtEnd() const {
    return currentOffset_ >= fileSize_;
}

bool ChunkReader::ValidateChunkStructure() const {
    if (discoveredChunks_.empty()) {
        return false;
    }
    
    // Check for proper End chunk termination
    bool hasEndChunk = false;
    for (const auto& chunk : discoveredChunks_) {
        if (chunk.IsEndMarker()) {
            hasEndChunk = true;
            break;
        }
    }
    
    if (!hasEndChunk) {
        ErrorHandler::PostEvent(0x6A, "No End chunk found");
        return false;
    }
    
    return true;
}

void ChunkReader::PrintChunkSummary() const {
    std::cout << "\\n=== Chunk Summary ===\\n";
    std::cout << "Total chunks discovered: " << discoveredChunks_.size() << "\\n";
    std::cout << "Chunk Details:\\n";
    std::cout << "  Type     | Size     | Name\\n";
    std::cout << "  ---------|----------|----------\\n";
    
    for (const auto& chunk : discoveredChunks_) {
        std::cout << "  0x" << std::hex << std::setfill('0') << std::setw(8) 
                  << chunk.rawID << std::dec << " | "
                  << std::setw(8) << chunk.size << " | "
                  << chunk.GetName() << "\\n";
    }
    
    std::cout << "===================\\n\\n";
}