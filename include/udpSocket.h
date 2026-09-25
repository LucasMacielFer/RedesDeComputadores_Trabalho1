#pragma once 
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

class udpSocket
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
    udpSocket();
    ~udpSocket();
    void bind(uint16_t port, bool ipv6);
    bool isBound() const;
    bool sendTo(const unsigned char* data, size_t length, const unsigned char* destIp, uint16_t destPort);
    bool recvFrom(unsigned char* buffer, size_t bufferSize, size_t& receivedLength, unsigned char* srcIp, uint16_t& srcPort);
    void close();
};