#pragma once 
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#elif __unix__
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
#else
    #error "Sistema operacional desconhecido. Este projeto suporta somente Windows e Unix."
#endif

class UdpSocket
{
private:
    static int instanceCount;

#ifdef _WIN32
    SOCKET sock;
#else
    int sock;
#endif
    uint16_t port;
    bool isIpv6;
    bool socketIsBound;

public:
    UdpSocket();
    ~UdpSocket();
    void bind(uint16_t port, bool ipv6);
    bool isBound() const;
    bool sendTo(const uint8_t* data, size_t length, const uint8_t* destIp, uint16_t destPort);
    bool recvFrom(uint8_t* buffer, size_t bufferSize, size_t& receivedLength, uint8_t* srcIp, uint16_t& srcPort);
    void close();
};