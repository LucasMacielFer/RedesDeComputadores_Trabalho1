#pragma once
#include <cstdint>
#include <filesystem>
#include <vector>

namespace FileTransfer
{
    class FileAssembler
    {
    private:
        bool metadataReceived;
        uint64_t expectedSize;
        uint32_t expectedCrc32;
        std::vector<uint8_t> buffer;
        
    public:
        FileAssembler();
        ~FileAssembler();

        void setMetadata(uint64_t totalSize, uint32_t expectedCrc32);
        void appendChunk(const std::vector<uint8_t>& data);

        bool hasMetadata() const;
        uint64_t getBytesReceived() const;
        uint64_t getExpectedSize() const;
        bool isComplete() const;

        bool verifyIntegrity() const;
        bool saveTo(const std::filesystem::path& outputPath) const;
    };
}
