#include "../include/udpSocket.h"

udpSocket::udpSocket() : port(0), isIpv6(false) 
{
    socketIsBound = false;
    memset(ipAddress, 0, sizeof(ipAddress));
    port = 0;
    isIpv6 = false;

    // TODO: Implementar a funcao de inicializacao do socket
#ifdef _WIN32
    // TODO
#elif __unix__
    // TODO
#endif
}

udpSocket::~udpSocket() {
    close();
}

void udpSocket::bind(const unsigned char* ip, uint16_t port, bool ipv6)
{
    // TODO: Implementar a funcao de bind do socket
#ifdef _WIN32
    // TODO
#elif __unix__
    // TODO
#endif
}

bool udpSocket::isBound() const
{
    return socketIsBound;
}

void udpSocket::setSocketOptions()
{
    // TODO: Implementar opcoes do socket
#ifdef _WIN32
    // TODO
#elif __unix__
    // TODO
#endif
}

void udpSocket::sendTo(const unsigned char* data, size_t length)
{
    // TODO: Implementar envio de dados
#ifdef _WIN32
    // TODO
#elif __unix__
    // TODO
#endif
}

void udpSocket::recvFrom(unsigned char* buffer, size_t bufferSize, size_t& receivedLength)
{
    // TODO: Implementar recepcao de dados
#ifdef _WIN32
    // TODO
#elif __unix__
    // TODO
#endif
}

void udpSocket::close()
{
    // TODO: Implementar fechamento do socket
#ifdef _WIN32
    // TODO
#elif __unix__
    // TODO
#endif
}