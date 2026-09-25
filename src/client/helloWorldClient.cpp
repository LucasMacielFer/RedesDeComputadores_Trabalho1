#include "udpSocket.h"
#include <cstdio>
 
int main()
{
    unsigned char localIp[4] = {127, 0, 0, 1};
    unsigned char destIp[4]  = {127, 0, 0, 1};
 
    udpSocket sender;
    sender.bind(2004, false);
 
    if (!sender.isBound())
    {
        printf("Falha ao dar bind na porta 2004\n");
        return 1;
    }
 
    const char* msg = "Hello, world!";
 
    bool ok = sender.sendTo((const unsigned char*)msg, strlen(msg), destIp, 2005);
 
    if (ok)
    {
        printf("Mensagem enviada: %s\n", msg);
    }
    else
    {
        printf("Falha ao enviar mensagem\n");
    }
 
    sender.close();
    return 0;
}
