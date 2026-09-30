#pragma once
#include "connection/connection.h"
#include "network/udpSocket.h"
#include "protocol/serializer.h"
#include "utils/ipFormatter.h"
#include <functional>

namespace Connection
{
    class ConnectionManager
    {
    public:
        using DataCallback = std::function<void(Connection&, const std::vector<uint8_t>&)>;

    private:
        std::vector<Connection*> connections;
        Network::UdpSocket udpSocket4;
        Network::UdpSocket udpSocket6;
        DataCallback dataCallback;

    public:
        ConnectionManager(uint16_t ipv4Port, uint16_t ipv6Port);
        ~ConnectionManager();
        void onDataReceived(const uint8_t* data, size_t length, const Network::Endpoint& peerEndpoint);
        void setOnDataReceived(DataCallback callback);
        void run();

    private:
        Connection* findConnection(const Network::Endpoint& peerEndpoint);
        Connection* createConnection(const Network::Endpoint& peerEndpoint);
    };
}