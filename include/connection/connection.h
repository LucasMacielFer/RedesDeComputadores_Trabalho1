#pragma once
#include "udpSocket.h"
#include "protocol/serializer.h"
#include <vector>
#include <chrono>

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
        Network::Endpoint peerEndpoint;
        ConnectionState state;

        uint32_t nextSendSequence;
        uint32_t nextReceiveSequence;

        bool waitingAck;
        uint32_t pendingSequence;

        std::vector<uint8_t> pendingPacket;

        std::chrono::steady_clock::time_point timerStart;

        bool hasRttSample;
        std::chrono::milliseconds srtt;
        std::chrono::milliseconds rttvar;
        std::chrono::milliseconds rto;

        bool packetWasRetransmitted;

    public:
        Connection(const Network::Endpoint peerEndpoint);
        ~Connection();
        const Network::Endpoint& getPeerEndpoint() const;
        ConnectionState getState() const;
        void handleDataReceived(const uint8_t* data, size_t length);
        bool sendData(const uint8_t* data, size_t length);
        void update();

    private:
        void handleSegment(const Protocol::Segment& segment);
        void sendSegment(const Protocol::Segment& segment, Network::UdpSocket& udpSocket);
        bool sendAck(uint32_t sequence);
        bool retransmitPending();
        void startTimer();
        void stopTimer();
        bool timeoutExpired() const;
        void updateRto(std::chrono::milliseconds rtt);
    };
}