#pragma once 
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
    unsigned char ipAddress[16];
    uint16_t port;
    bool isIpv6;
    bool socketIsBound;

public:
    udpSocket();
    ~udpSocket();
    void bind(const unsigned char* ip, uint16_t port, bool ipv6);
    bool isBound() const;
    void setSocketOptions();
    void sendTo(const unsigned char* data, size_t length);
    void recvFrom(unsigned char* buffer, size_t bufferSize, size_t& receivedLength);
    void close();
};