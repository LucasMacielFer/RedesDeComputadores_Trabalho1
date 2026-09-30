#pragma once
#include "udpSocket.h"
#include "protocol/serializer.h"
#include <vector>

namespace Connection
{
    enum ConnectionState
    {
        CLOSED,
        LISTEN,
        SYN_SENT,
        SYN_RECEIVED,
        ESTABLISHED,
        FIN_WAIT,
        CLOSE_WAIT,
        LAST_ACK,
        TIME_WAIT
    };

    class Connection
    {
    private:
        uint16_t peerPort;
        uint8_t peerIp[16];
        bool peerIpv6;
        uint32_t connectionId;
        ConnectionState state;

    public:
        Connection(const uint8_t* ip, uint16_t port, bool ipv6);
        ~Connection();

        const uint16_t getPeerPort() const;
        const uint8_t* getPeerIp() const;
        const bool isPeerIpv6() const;
        const uint32_t getConnectionId() const;
        
        void handleSegment(const Protocol::Segment& segment);
        void sendSegment(const Protocol::Segment& segment, UdpSocket& udpSocket);
        void setConnectionId(uint32_t id);
    };
}