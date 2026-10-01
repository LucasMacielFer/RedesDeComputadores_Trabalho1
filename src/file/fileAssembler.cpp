#include "file/fileAssembler.h"
#include "protocol/serializer.h"
#include <fstream>

namespace FileTransfer
{
    FileAssembler::FileAssembler() : 
    metadataReceived(false), 
    expectedSize(0), 
    expectedCrc32(0)
    {
    }

    FileAssembler::~FileAssembler()
    {
    }

    void FileAssembler::setMetadata(uint64_t totalSize, uint32_t crc)
    {
        expectedSize = totalSize;
        expectedCrc32 = crc;
        metadataReceived = true;
        buffer.reserve(static_cast<size_t>(totalSize));
    }

    void FileAssembler::appendChunk(const std::vector<uint8_t>& data)
    {
        buffer.insert(buffer.end(), data.begin(), data.end());
    }

    bool FileAssembler::hasMetadata() const
    {
        return metadataReceived;
    }

    uint64_t FileAssembler::getBytesReceived() const
    {
        return buffer.size();
    }

    uint64_t FileAssembler::getExpectedSize() const
    {
        return expectedSize;
    }

    bool FileAssembler::isComplete() const
    {
        return metadataReceived && buffer.size() >= expectedSize;
    }

    bool FileAssembler::verifyIntegrity() const
    {
        return Protocol::Serializer::calculateCrc32(buffer) == expectedCrc32;
    }

    bool FileAssembler::saveTo(const std::filesystem::path& outputPath) const
    {
        std::ofstream out(outputPath, std::ios::binary);
        if (!out.is_open())
            return false;

        out.write(reinterpret_cast<const char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
        return out.good();
    }
}
