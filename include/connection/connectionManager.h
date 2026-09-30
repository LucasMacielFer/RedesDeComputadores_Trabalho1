#pragma once
#include "connection/connection.h"
#include "network/udpSocket.h"
#include "protocol/serializer.h"
#include "utils/ipFormatter.h"

namespace Connection
{
    class ConnectionManager
    {
    private:
        std::vector<Connection*> connections;
        Network::UdpSocket udpSocket4;
        Network::UdpSocket udpSocket6;

    public:
        ConnectionManager(uint16_t port);
        ~ConnectionManager();
        void onDataReceived(const uint8_t* data, size_t length, const Network::Endpoint& peerEndpoint);

    private:
        Connection* findConnection(const Network::Endpoint& peerEndpoint);
        Connection* createConnection(const Network::Endpoint& peerEndpoint);
    };
}