#include "../include/udpSocket.h"

int udpSocket::instanceCount = 0;

udpSocket::udpSocket()
{
    memset(localIpAddress, 0, sizeof(localIpAddress));
 
#ifdef _WIN32
    sock = INVALID_SOCKET;
#endif

    if (instanceCount == 0)
    {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    }
    instanceCount++;
}

udpSocket::~udpSocket() {
    close();
 
    instanceCount--;
    if (instanceCount == 0)
    {
#ifdef _WIN32
        WSACleanup();
#endif
    }
}

void udpSocket::bind(const unsigned char* ip, uint16_t port, bool ipv6)
{
    sock = ::socket(ipv6 ? AF_INET6 : AF_INET, SOCK_DGRAM, IPPROTO_UDP);

#ifdef _WIN32
    if (sock == INVALID_SOCKET)
#else
    if (sock < 0)
#endif
    {
        printf("ERRO: Erro ao criar socket\n");
        return;
    }
 
    if (ipv6)
    {
        sockaddr_in6 addr{};
        addr.sin6_family = AF_INET6;
        addr.sin6_port = htons(port);
        memcpy(&addr.sin6_addr, ip, 16);
 
        if (::bind(sock, (sockaddr*)&addr, sizeof(addr)) != 0)
        {
            printf("ERRO: Erro ao dar bind (IPv6)\n");
            return;
        }
    }
    else
    {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        memcpy(&addr.sin_addr, ip, 4);
 
        if (::bind(sock, (sockaddr*)&addr, sizeof(addr)) != 0)
        {
            printf("ERRO: Erro ao dar bind (IPv4)\n");
            return;
        }
    }
 
    isIpv6 = ipv6;
    this->port = port;
    memset(localIpAddress, 0, sizeof(localIpAddress));
    memcpy(localIpAddress, ip, ipv6 ? 16 : 4);
    socketIsBound = true;
}

bool udpSocket::isBound() const
{
    return socketIsBound;
}

bool udpSocket::sendTo(const unsigned char* data, size_t length, const unsigned char* destIp, uint16_t destPort)
{
    if (!socketIsBound)
    {
        printf("ERRO: Socket sem bind\n");
        return false;
    }
 
    int result;
 
    if (isIpv6)
    {
        sockaddr_in6 addr{};
        addr.sin6_family = AF_INET6;
        addr.sin6_port = htons(destPort);
        memcpy(&addr.sin6_addr, destIp, 16);
 
        result = ::sendto(sock, (const char*)data, (int)length, 0, (sockaddr*)&addr, sizeof(addr));
    }
    else
    {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(destPort);
        memcpy(&addr.sin_addr, destIp, 4);
 
        result = ::sendto(sock, (const char*)data, (int)length, 0, (sockaddr*)&addr, sizeof(addr));
    }
 
    if (result < 0)
    {
        printf("ERRO: Erro ao enviar dados\n");
        return false;
    }
    return true;
}

bool udpSocket::recvFrom(unsigned char* buffer, size_t bufferSize, size_t& receivedLength, unsigned char* srcIp, uint16_t& srcPort)
{
    if (!socketIsBound)
    {
        printf("ERRO: Socket sem bind\n");
        return false;
    }
 
    sockaddr_storage fromAddr{};
    socklen_t fromLen = sizeof(fromAddr);
 
    int result = ::recvfrom(sock, (char*)buffer, (int)bufferSize, 0, (sockaddr*)&fromAddr, &fromLen);
    if (result < 0)
    {
        printf("ERRO: Erro ao receber dados\n");
        return false;
    }
 
    receivedLength = (size_t)result;
 
    memset(srcIp, 0, 16);
    if (fromAddr.ss_family == AF_INET6)
    {
        sockaddr_in6* addr6 = (sockaddr_in6*)&fromAddr;
        memcpy(srcIp, &addr6->sin6_addr, 16);
        srcPort = ntohs(addr6->sin6_port);
    }
    else
    {
        sockaddr_in* addr4 = (sockaddr_in*)&fromAddr;
        memcpy(srcIp, &addr4->sin_addr, 4);
        srcPort = ntohs(addr4->sin_port);
    }
 
    return true;
}

void udpSocket::close()
{
#ifdef _WIN32
    if (sock != INVALID_SOCKET)
    {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
#else
    if (sock >= 0)
    {
        ::close(sock);
        sock = -1;
    }
#endif
    socketIsBound = false;
}