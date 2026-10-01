#pragma once
#include "connection/connection.h"
#include "network/udpSocket.h"
#include "file/fileAssembler.h"
#include <chrono>
#include <cstdint>
#include <string>

#define LOSS_PROBABILITY 0.1
#define CORRUPTION_PROBABILITY 0.1

namespace FileTransfer
{
    class Client
    {
    private:
        static constexpr std::chrono::milliseconds POLL_INTERVAL{100};

        Network::UdpSocket socket;
        Connection::Connection connection;
        FileAssembler assembler;

        bool simulatePacketLoss;
        bool simulateCorruption;
        bool errorReceived = false;
        std::string errorText;

    public:
        Client(const Network::Endpoint& serverEndpoint, bool simulatePacketLoss, bool simulateCorruption);
        bool requestFile(const std::string& remoteFilename, const std::string& localPath);

    private:
        void onDataReceived(const std::vector<uint8_t>& data);
        void pump();
        void drainUntilClosed();
    };
}
