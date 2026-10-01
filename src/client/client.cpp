#include "client/client.h"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>

namespace FileTransfer
{
    namespace
    {
        void ensureRandSeeded()
        {
            static bool seeded = false;
            if (!seeded)
            {
                srand(static_cast<unsigned int>(time(nullptr)));
                seeded = true;
            }
        }

        bool shouldSimulateInstability(double probability)
        {
            ensureRandSeeded();
            return (static_cast<double>(rand()) / RAND_MAX) < probability;
        }

        bool flipRandomBit(uint8_t* data, size_t length)
        {
            constexpr size_t HEADER_SIZE = sizeof(Protocol::Header);
            if (length <= HEADER_SIZE)
                return false;

            ensureRandSeeded();

            const size_t byteIndex = HEADER_SIZE + static_cast<size_t>(rand()) % (length - HEADER_SIZE);
            const int bitIndex = rand() % 8;

            data[byteIndex] ^= static_cast<uint8_t>(1u << bitIndex);
            return true;
        }
    }

    Client::Client(const Network::Endpoint& serverEndpoint, bool simulatePacketLoss, bool simulateCorruption) :
        socket(serverEndpoint.isIpv6),
        connection(serverEndpoint, socket),
        simulatePacketLoss(simulatePacketLoss),
        simulateCorruption(simulateCorruption)
    {
        connection.setOnDataReceived([this](const std::vector<uint8_t>& data) { onDataReceived(data); });
    }

    void Client::onDataReceived(const std::vector<uint8_t>& data)
    {
        if (data.empty())
            return;

        const uint8_t type = data[0];

        if (type == Protocol::METADATA)
        {
            uint64_t totalSize;
            uint32_t crc;
            if (data.size() < 1 + sizeof(totalSize) + sizeof(crc))
                return;

            memcpy(&totalSize, data.data() + 1, sizeof(totalSize));
            memcpy(&crc, data.data() + 1 + sizeof(totalSize), sizeof(crc));

            assembler.setMetadata(totalSize, crc);
            std::cout << "[CLIENT] Metadados recebidos: " << totalSize << " bytes, crc32=0x" << std::hex << crc << std::dec << std::endl;
        }
        else if (type == Protocol::DATA)
        {
            assembler.appendChunk(std::vector<uint8_t>(data.begin() + 1, data.end()));
        }
        else if (type == Protocol::FILE_ERROR)
        {
            errorReceived = true;
            const uint8_t reason = data.size() > 1 ? data[1] : 0;
            errorText = (reason == Protocol::NOT_FOUND) ? "Arquivo nao encontrado" : "Permissao negada";
        }
    }

    void Client::pump()
    {
        if (socket.waitForData(POLL_INTERVAL))
        {
            uint8_t buffer[2048];
            size_t receivedLength;
            Network::Endpoint srcEndpoint;

            if (socket.recvFrom(buffer, sizeof(buffer), receivedLength, srcEndpoint))
            {
                if (simulatePacketLoss && shouldSimulateInstability(LOSS_PROBABILITY))
                {
                    std::cout << "[SIM] Pacote de " << receivedLength << " bytes descartado (perda simulada)." << std::endl;
                }
                else
                {
                    if (simulateCorruption && shouldSimulateInstability(CORRUPTION_PROBABILITY) && flipRandomBit(buffer, receivedLength))
                        std::cout << "[SIM] Bit invertido na secao de dados (corrupcao simulada)." << std::endl;

                    connection.handleDataReceived(buffer, receivedLength);
                }
            }
        }

        connection.update();
    }

    void Client::drainUntilClosed()
    {
        while (connection.getState() != Connection::CLOSED)
            pump();
    }

    bool Client::requestFile(const std::string& remoteFilename, const std::string& localPath)
    {
        if (!connection.connect())
        {
            std::cerr << "[CLIENT] Falha ao iniciar a conexao." << std::endl;
            return false;
        }

        while (connection.getState() == Connection::SYN_SENT)
            pump();

        if (connection.getState() != Connection::ESTABLISHED)
        {
            std::cerr << "[CLIENT] Nao foi possivel estabelecer a conexao." << std::endl;
            return false;
        }

        std::vector<uint8_t> request;
        request.push_back(Protocol::REQUEST);
        request.insert(request.end(), remoteFilename.begin(), remoteFilename.end());
        connection.sendData(request.data(), request.size());

        while (!errorReceived && connection.getState() != Connection::CLOSED &&
               !(assembler.hasMetadata() && assembler.isComplete()))
            pump();

        if (errorReceived)
        {
            std::cerr << "[CLIENT] Erro do servidor: " << errorText << std::endl;
            drainUntilClosed();
            return false;
        }

        if (!assembler.hasMetadata() || !assembler.isComplete())
        {
            std::cerr << "[CLIENT] Conexao encerrada antes de completar a transferencia (" << assembler.getBytesReceived()
                       << "/" << assembler.getExpectedSize() << " bytes)." << std::endl;
            return false;
        }

        if (!assembler.verifyIntegrity())
        {
            std::cerr << "[CLIENT] Falha de integridade: CRC32 nao confere." << std::endl;
            return false;
        }

        if (!assembler.saveTo(localPath))
        {
            std::cerr << "[CLIENT] Falha ao salvar o arquivo em " << localPath << std::endl;
            return false;
        }

        std::cout << "[CLIENT] Arquivo salvo com sucesso em " << localPath << " (" << assembler.getBytesReceived() << " bytes)." << std::endl;

        connection.close();
        drainUntilClosed();

        return true;
    }
}
