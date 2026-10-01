#include "server/server.h"
#include <chrono>

namespace FileTransfer
{
    namespace
    {
        void appendUint64(std::vector<uint8_t>& out, uint64_t value)
        {
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
            out.insert(out.end(), bytes, bytes + sizeof(value));
        }

        void appendUint32(std::vector<uint8_t>& out, uint32_t value)
        {
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
            out.insert(out.end(), bytes, bytes + sizeof(value));
        }
    }

    Server::Server(uint16_t ipv4Port, uint16_t ipv6Port, std::filesystem::path bucketRoot) :
        fileSerializer(std::move(bucketRoot)),
        connectionManager(ipv4Port, ipv6Port)
    {
        connectionManager.setOnDataReceived([this](Connection::Connection& connection, const std::vector<uint8_t>& data)
        {
            onRequestReceived(connection, data);
        });
    }

    Server::~Server()
    {
    }

    void Server::run()
    {
        connectionManager.run();
    }

    void Server::onRequestReceived(Connection::Connection& connection, const std::vector<uint8_t>& data)
    {
        if (data.empty() || data[0] != Protocol::REQUEST)
            return;

        const std::string filename(data.begin() + 1, data.end());

        Protocol::FileErrorReason errorReason{};
        std::shared_ptr<FileTransferSession> session = fileSerializer.open(filename, errorReason);

        if (!session)
        {
            std::cout << "[SERVER] Pedido de \"" << filename << "\" recusado." << std::endl;
            sendError(connection, errorReason);
            return;
        }

        std::cout << "[SERVER] Enviando \"" << filename << "\" (" << session->getFileSize() << " bytes)" << std::endl;

        std::vector<uint8_t> metadata;
        metadata.push_back(Protocol::METADATA);
        appendUint64(metadata, session->getFileSize());
        appendUint32(metadata, session->getFileCrc32());

        connection.sendData(metadata.data(), metadata.size());
        connection.setOnIdle([this, &connection, session]() { pumpNextChunk(connection, session); });
    }

    void Server::pumpNextChunk(Connection::Connection& connection, std::shared_ptr<FileTransferSession> session)
    {
        if (session->isFinished())
        {
            std::cout << "[SERVER] Transferencia concluida do lado do servidor; aguardando confirmacao do cliente." << std::endl;
            return;
        }

        const std::vector<uint8_t> chunk = session->readNextChunk(MAX_CHUNK_PAYLOAD);

        std::vector<uint8_t> message;
        message.reserve(chunk.size() + 1);
        message.push_back(Protocol::DATA);
        message.insert(message.end(), chunk.begin(), chunk.end());

        connection.sendData(message.data(), message.size());
        connection.setOnIdle([this, &connection, session]() { pumpNextChunk(connection, session); });
    }

    void Server::sendError(Connection::Connection& connection, Protocol::FileErrorReason reason)
    {
        std::vector<uint8_t> message;
        message.push_back(Protocol::FILE_ERROR);
        message.push_back(static_cast<uint8_t>(reason));

        connection.sendData(message.data(), message.size());
        connection.setOnIdle([&connection]() { connection.close(); });
    }
}
