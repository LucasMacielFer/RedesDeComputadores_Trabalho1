#include "../include/udpSocket.h"

int UdpSocket::instanceCount = 0;

UdpSocket::UdpSocket()
{ 
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

UdpSocket::~UdpSocket() {
    close();
 
    instanceCount--;
    if (instanceCount == 0)
    {
#ifdef _WIN32
        WSACleanup();
#endif
    }
}

void UdpSocket::bind(uint16_t port, bool ipv6)
{
    sock = ::socket(ipv6 ? AF_INET6 : AF_INET, SOCK_DGRAM, IPPROTO_UDP);

#ifdef _WIN32
    if (sock == INVALID_SOCKET)
#else
    if (sock < 0)
#endif
    {
        std::cerr << "ERRO: Erro ao criar socket" << std::endl;
        return;
    }
 
    if (ipv6)
    {
        sockaddr_in6 addr{};
        addr.sin6_family = AF_INET6;
        addr.sin6_port = htons(port);
        addr.sin6_addr = in6addr_any;
 
        if (::bind(sock, (sockaddr*)&addr, sizeof(addr)) != 0)
        {
            std::cerr << "ERRO: Erro ao dar bind (IPv6)" << std::endl;
            return;
        }
    }
    else
    {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;
 
        if (::bind(sock, (sockaddr*)&addr, sizeof(addr)) != 0)
        {
            std::cerr << "ERRO: Erro ao dar bind (IPv4)" << std::endl;
            return;
        }
    }
 
    isIpv6 = ipv6;
    this->port = port;
    socketIsBound = true;
}

bool UdpSocket::isBound() const
{
    return socketIsBound;
}

bool UdpSocket::sendTo(const uint8_t* data, size_t length, const uint8_t* destIp, uint16_t destPort)
{
    if (!socketIsBound)
    {
        std::cerr << "ERRO: Socket sem bind" << std::endl;
        return false;
    }
 
    int result;
 
    if (isIpv6)
    {
        sockaddr_in6 addr{};
        addr.sin6_family = AF_INET6;
        addr.sin6_port = htons(destPort);
        memcpy(&addr.sin6_addr, destIp, 16);
 
        result = ::sendto(sock, (const uint8_t*)data, (int)length, 0, (sockaddr*)&addr, sizeof(addr));
    }
    else
    {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(destPort);
        memcpy(&addr.sin_addr, destIp, 4);
 
        result = ::sendto(sock, (const uint8_t*)data, (int)length, 0, (sockaddr*)&addr, sizeof(addr));
    }
 
    if (result < 0)
    {
        std::cerr << "ERRO: Erro ao enviar dados" << std::endl;
        return false;
    }
    return true;
}

bool UdpSocket::recvFrom(uint8_t* buffer, size_t bufferSize, size_t& receivedLength, uint8_t* srcIp, uint16_t& srcPort)
{
    if (!socketIsBound)
    {
        std::cerr << "ERRO: Socket sem bind" << std::endl;
        return false;
    }
 
    sockaddr_storage fromAddr{};
    socklen_t fromLen = sizeof(fromAddr);
 
    int result = ::recvfrom(sock, (uint8_t*)buffer, (int)bufferSize, 0, (sockaddr*)&fromAddr, &fromLen);
    if (result < 0)
    {
        std::cerr << "ERRO: Erro ao receber dados" << std::endl;
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

void UdpSocket::close()
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