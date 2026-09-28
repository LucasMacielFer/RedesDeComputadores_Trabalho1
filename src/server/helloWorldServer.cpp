#include "udpSocket.h"
#include <cstdio>
 
int main()
{
    unsigned char localIp[4] = {127, 0, 0, 1};
 
    UdpSocket receiver;
    receiver.bind(2005, false);
 
    if (!receiver.isBound())
    {
        printf("Falha ao dar bind na porta 2005\n");
        return 1;
    }
 
    printf("Aguardando mensagem na porta 2005...\n");
 
    unsigned char buffer[1024];
    size_t receivedLength = 0;
    unsigned char srcIp[16];
    uint16_t srcPort = 0;
 
    bool ok = receiver.recvFrom(buffer, sizeof(buffer) - 1, receivedLength, srcIp, srcPort);
 
    if (ok)
    {
        buffer[receivedLength] = '\0'; // transforma em string valida pra printf
        printf("Recebido de %d.%d.%d.%d:%d -> %s\n",
               srcIp[0], srcIp[1], srcIp[2], srcIp[3], srcPort, buffer);
    }
    else
    {
        printf("Falha ao receber mensagem\n");
    }
 
    receiver.close();
    return 0;
}