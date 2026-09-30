#pragma once
#include "network/udpSocket.h"
#include "protocol/serializer.h"
#include <vector>
#include <chrono>
#include <functional>

namespace Connection
{
    enum ConnectionState
    {
        CLOSED,
        SYN_SENT,
        SYN_RECEIVED,
        ESTABLISHED,
        CLOSING
    };

    class Connection
    {
    public:
        using DataCallback = std::function<void(const std::vector<uint8_t>&)>;
        using IdleCallback = std::function<void()>;

    private:
        static constexpr std::chrono::milliseconds INITIAL_RTO{1000};
        static constexpr std::chrono::milliseconds MIN_RTO{200};
        static constexpr std::chrono::milliseconds MAX_RTO{5000};
        static constexpr int MAX_RETRIES = 3;

        Network::UdpSocket& udpSocket;
        Network::Endpoint peerEndpoint;
        ConnectionState state;

        uint32_t nextSendSequence;
        uint32_t nextReceiveSequence;

        bool waitingAck;
        uint32_t pendingSequence;
        std::vector<uint8_t> pendingDataPacket;

        int retryCount;
        bool packetWasRetransmitted;

        bool timerRunning;
        std::chrono::steady_clock::time_point timerStart;

        bool hasRttSample;
        std::chrono::milliseconds srtt;
        std::chrono::milliseconds rttvar;
        std::chrono::milliseconds rto;

        DataCallback dataReceivedCallback;
        IdleCallback idleCallback;

    public:
        Connection(const Network::Endpoint peerEndpoint, Network::UdpSocket& udpSocket);
        ~Connection();
        const Network::Endpoint& getPeerEndpoint() const;
        ConnectionState getState() const;

        bool isIdle() const;
        void setOnDataReceived(DataCallback callback);
        void setOnIdle(IdleCallback callback);

        bool connect();
        void handleDataReceived(const uint8_t* data, size_t length);
        bool sendData(const uint8_t* data, size_t length);
        bool close();
        void update();

    private:
        void handleSegment(const Protocol::Segment& segment);
        void handleAck(const Protocol::Header& header);
        void handleNack(const Protocol::Header& header);
        void handleDataSegment(const Protocol::Segment& segment);
        void handleSyn(const Protocol::Segment& segment);
        void handleFin(const Protocol::Segment& segment);

        bool sendSegment(const Protocol::Segment& segment);
        bool sendAck(uint32_t sequence);
        bool sendNack(uint32_t sequence);

        void startTimer();
        void stopTimer();
        bool timeoutExpired() const;
        bool retransmitPending();
        void giveUp();

        void updateRto(std::chrono::milliseconds rtt);
    };
}
