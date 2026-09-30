#pragma once
#include "connection/connection.h"
#include "udpSocket.h"
#include "protocol/serializer.h"
#include "utils/ipFormatter.h"

namespace Connection
{
    class ConnectionManager
    {
    private:
        std::vector<Connection*> connections;
        UdpSocket udpSocket4;
        UdpSocket udpSocket6;

    public:
        ConnectionManager(uint16_t port);
        ~ConnectionManager();
        void onDataReceived(const uint8_t* data, size_t length, const uint8_t* srcIp, uint16_t srcPort);

    private:
        Connection* findConnection(const uint8_t* peerIp, uint16_t peerPort, bool isIpv6, uint32_t connectionId);
        Connection* createConnection(const uint8_t* peerIp, uint16_t peerPort, bool isIpv6, uint32_t connectionId);
    };
}