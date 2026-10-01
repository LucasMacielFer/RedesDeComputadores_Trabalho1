#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <chrono>
#include "network/endpoint.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#elif __unix__
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <unistd.h>
#else
    #error "Sistema operacional desconhecido. Este projeto suporta somente Windows e Unix."
#endif

namespace Network
{
    class UdpSocket
    {
    private:
        static int instanceCount;

    #ifdef _WIN32
        SOCKET sock;
    #else
        int sock;
    #endif
        Endpoint localEndpoint;
        bool socketIsBound;

    public:
        UdpSocket(bool ipv6);
        ~UdpSocket();
        void bind(uint16_t port);
        bool isBound() const;
        bool sendTo(const uint8_t* data, size_t length, const Endpoint& destEndpoint);
        bool recvFrom(uint8_t* buffer, size_t bufferSize, size_t& receivedLength, Endpoint& srcEndpoint);
        bool waitForData(std::chrono::milliseconds timeout) const;

        static void waitForEither(const UdpSocket& first, const UdpSocket& second, std::chrono::milliseconds timeout, bool& firstReady, bool& secondReady);

        void close();

    private:
        void updateLocalPort();
    };
}