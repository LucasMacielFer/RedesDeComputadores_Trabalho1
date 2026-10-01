#include "server/server.h"

#define PORT_V4 2005
#define PORT_V6 2026

/*
    Universidade Tecnológica Federal do Paraná - UTFPR
    Departamento Acadêmico de Informática - DAINF
    ICSR30 - Redes de Computadores

    Trabalho de Programação 1

    Lucas Maciel Ferreira - 2556596
*/ 

int main()
{
    FileTransfer::Server server(PORT_V4, PORT_V6, "server_bucket");

    std::cout << "[SERVER] Servidor ativo (IPv4 " << PORT_V4 << ", IPv6 " << PORT_V6 << ")..." << std::endl;
    server.run();

    return 0;
}
