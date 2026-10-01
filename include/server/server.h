#pragma once
#include "connection/connectionManager.h"
#include "file/fileSerializer.h"
#include <cstdint>
#include <filesystem>
#include <memory>

#define MAX_CHUNK_PAYLOAD Protocol::Serializer::MAX_PAYLOAD_SIZE - 1

namespace FileTransfer
{
    class Server
    {
    private:
        FileSerializer fileSerializer;
        Connection::ConnectionManager connectionManager;
    
    public:
        Server(uint16_t ipv4Port, uint16_t ipv6Port, std::filesystem::path bucketRoot);
        ~Server();
        void run();

    private:
        void onRequestReceived(Connection::Connection& connection, const std::vector<uint8_t>& data);
        void pumpNextChunk(Connection::Connection& connection, std::shared_ptr<FileTransferSession> session);
        static void sendError(Connection::Connection& connection, Protocol::FileErrorReason reason);
    };
}
