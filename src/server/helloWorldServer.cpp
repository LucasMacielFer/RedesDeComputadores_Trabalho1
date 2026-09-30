#include "connection/connectionManager.h"
#include <string>

#define HELLO_PORT_V4 6000
#define HELLO_PORT_V6 6001

int main()
{
    Connection::ConnectionManager manager(HELLO_PORT_V4, HELLO_PORT_V6);

    manager.setOnDataReceived([](Connection::Connection& connection, const std::vector<uint8_t>& data)
    {
        const std::string message(data.begin(), data.end());
        std::cout << "[APP] Mensagem recebida: " << message << std::endl;

        const std::string reply = "Hello Client";
        connection.sendData(reinterpret_cast<const uint8_t*>(reply.data()), reply.size());
        connection.setOnIdle([&connection]()
        {
            connection.close();
        });
    });

    std::cout << "[APP] Servidor aguardando (IPv4 " << HELLO_PORT_V4 << ", IPv6 " << HELLO_PORT_V6 << ")..." << std::endl;
    manager.run();

    return 0;
}
