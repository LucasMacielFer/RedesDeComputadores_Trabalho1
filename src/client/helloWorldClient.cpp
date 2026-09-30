#include "connection/connection.h"
#include "network/udpSocket.h"
#include <string>
#include <chrono>

namespace
{
    constexpr uint16_t HELLO_PORT = 6000;
    constexpr std::chrono::milliseconds POLL_INTERVAL(100);

    void pump(Connection::Connection& connection, Network::UdpSocket& socket)
    {
        if (socket.waitForData(POLL_INTERVAL))
        {
            uint8_t buffer[2048];
            size_t receivedLength;
            Network::Endpoint srcEndpoint;

            if (socket.recvFrom(buffer, sizeof(buffer), receivedLength, srcEndpoint))
                connection.handleDataReceived(buffer, receivedLength);
        }

        connection.update();
    }
}

int main()
{
    Network::Endpoint serverEndpoint{};
    serverEndpoint.isIpv6 = false;
    serverEndpoint.ip[0] = 127;
    serverEndpoint.ip[1] = 0;
    serverEndpoint.ip[2] = 0;
    serverEndpoint.ip[3] = 1;
    serverEndpoint.port = HELLO_PORT;

    Network::UdpSocket socket(false);
    Connection::Connection connection(serverEndpoint, socket);

    connection.setOnDataReceived([](const std::vector<uint8_t>& data)
    {
        const std::string message(data.begin(), data.end());
        std::cout << "[APP] Servidor respondeu: " << message << std::endl;
    });

    if (!connection.connect())
    {
        std::cerr << "[APP] Falha ao iniciar a conexao." << std::endl;
        return 1;
    }

    while (connection.getState() == Connection::SYN_SENT)
        pump(connection, socket);

    if (connection.getState() != Connection::ESTABLISHED)
    {
        std::cerr << "[APP] Nao foi possivel estabelecer a conexao." << std::endl;
        return 1;
    }

    const std::string message = "Hello World";
    connection.sendData(reinterpret_cast<const uint8_t*>(message.data()), message.size());

    while (connection.getState() != Connection::CLOSED)
        pump(connection, socket);

    std::cout << "[APP] Conexao encerrada." << std::endl;
    return 0;
}
