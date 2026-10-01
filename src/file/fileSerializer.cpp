#include "file/fileSerializer.h"
#include "protocol/serializer.h"
#include <algorithm>
#include <system_error>

namespace FileTransfer
{
    namespace
    {
        bool isInside(const std::filesystem::path& bucketRoot, const std::filesystem::path& candidate)
        {
            const std::string root = bucketRoot.string();
            const std::string target = candidate.string();

            if (target == root)
                return true;

            return target.size() > root.size() &&
                   target.compare(0, root.size(), root) == 0 &&
                   target[root.size()] == std::filesystem::path::preferred_separator;
        }
    }

    FileTransferSession::FileTransferSession(std::ifstream file, uint64_t fileSize, uint32_t fileCrc32) :
        file(std::move(file)),
        fileSize(fileSize),
        fileCrc32(fileCrc32),
        bytesSent(0)
    {
    }

    FileTransferSession::~FileTransferSession()
    {
        if (file.is_open())
            file.close();
    }

    uint64_t FileTransferSession::getFileSize() const
    {
        return fileSize;
    }

    uint32_t FileTransferSession::getFileCrc32() const
    {
        return fileCrc32;
    }

    bool FileTransferSession::isFinished() const
    {
        return bytesSent >= fileSize;
    }

    std::vector<uint8_t> FileTransferSession::readNextChunk(size_t maxSize)
    {
        const uint64_t remaining = fileSize - bytesSent;
        const size_t toRead = static_cast<size_t>(std::min<uint64_t>(remaining, maxSize));

        std::vector<uint8_t> chunk(toRead);
        file.read(reinterpret_cast<char*>(chunk.data()), static_cast<std::streamsize>(toRead));
        bytesSent += toRead;

        return chunk;
    }

    FileSerializer::FileSerializer(std::filesystem::path bucketRoot) :
        bucketRoot(std::filesystem::weakly_canonical(bucketRoot))
    {
    }

    FileSerializer::~FileSerializer()
    {
    }

    std::shared_ptr<FileTransferSession> FileSerializer::open(const std::string& requestedName, Protocol::FileErrorReason& outError) const
    {
        if (requestedName.empty() || requestedName.find("..") != std::string::npos)
        {
            outError = Protocol::PERMISSION_DENIED;
            return nullptr;
        }

        const std::filesystem::path requested(requestedName);
        if (requested.is_absolute())
        {
            outError = Protocol::PERMISSION_DENIED;
            return nullptr;
        }

        std::error_code ec;
        const std::filesystem::path candidate = std::filesystem::weakly_canonical(bucketRoot / requested, ec);

        if (ec || !isInside(bucketRoot, candidate))
        {
            outError = Protocol::PERMISSION_DENIED;
            return nullptr;
        }

        if (!std::filesystem::exists(candidate, ec) || !std::filesystem::is_regular_file(candidate, ec))
        {
            outError = Protocol::NOT_FOUND;
            return nullptr;
        }

        std::ifstream file(candidate, std::ios::binary);
        if (!file.is_open())
        {
            outError = Protocol::NOT_FOUND;
            return nullptr;
        }

        const uint64_t fileSize = std::filesystem::file_size(candidate, ec);
        if (ec)
        {
            outError = Protocol::NOT_FOUND;
            return nullptr;
        }

        uint32_t crc = 0;
        std::vector<uint8_t> hashBuffer(HASH_CHUNK_SIZE);
        while (file)
        {
            file.read(reinterpret_cast<char*>(hashBuffer.data()), static_cast<std::streamsize>(hashBuffer.size()));
            const std::streamsize got = file.gcount();
            if (got <= 0)
                break;

            crc = Protocol::Serializer::calculateCrc32(hashBuffer.data(), static_cast<size_t>(got), crc);
        }

        file.clear();
        file.seekg(0);

        return std::make_shared<FileTransferSession>(std::move(file), fileSize, crc);
    }
}
