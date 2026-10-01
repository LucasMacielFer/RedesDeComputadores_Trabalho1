#pragma once
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include "protocol/protocol.h"

#define HASH_CHUNK_SIZE 65536

namespace FileTransfer
{
    class FileTransferSession
    {
    private:
        std::ifstream file;
        uint64_t fileSize;
        uint32_t fileCrc32;
        uint64_t bytesSent;

    public:
        FileTransferSession(std::ifstream file, uint64_t fileSize, uint32_t fileCrc32);
        ~FileTransferSession();

        uint64_t getFileSize() const;
        uint32_t getFileCrc32() const;
        bool isFinished() const;

        std::vector<uint8_t> readNextChunk(size_t maxSize);
    };

    class FileSerializer
    {
    private:
        std::filesystem::path bucketRoot;
    
    public:
        FileSerializer(std::filesystem::path bucketRoot);
        ~FileSerializer();
        std::shared_ptr<FileTransferSession> open(const std::string& requestedName, Protocol::FileErrorReason& outError) const;
    };
}
