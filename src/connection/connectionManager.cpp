#include "connection/connectionManager.h"
#include <algorithm>
#include <chrono>

namespace Connection
{
    ConnectionManager::ConnectionManager(uint16_t ipv4Port, uint16_t ipv6Port):
        udpSocket4(false),
        udpSocket6(true)
    {
        connections = std::vector<Connection*>();

        udpSocket4.bind(ipv4Port);
        if(!udpSocket4.isBound())
        {
            for(int i = 0; i < 3; ++i)
            {
                std::cerr << "ERRO: Falha ao vincular o socket UDP IPv4 a porta " << ipv4Port << ". Tentativa " << (i + 1) << " de 3." << std::endl;
                udpSocket4.bind(ipv4Port);

                if(udpSocket4.isBound())
                    break;
            }
        }

        if(!udpSocket4.isBound())
        {
            std::cerr << "ERRO: Falha ao vincular o socket UDP IPv4 a porta " << ipv4Port << " apos 3 tentativas." << std::endl;
            exit(EXIT_FAILURE);
        }

        udpSocket6.bind(ipv6Port);
        if(!udpSocket6.isBound())
        {
            for(int i = 0; i < 3; ++i)
            {
                std::cerr << "ERRO: Falha ao vincular o socket UDP IPv6 a porta " << ipv6Port << ". Tentativa " << (i + 1) << " de 3." << std::endl;
                udpSocket6.bind(ipv6Port);

                if(udpSocket6.isBound())
                    break;
            }
        }

        if(!udpSocket6.isBound())
        {
            std::cerr << "ERRO: Falha ao vincular o socket UDP IPv6 a porta " << ipv6Port << " apos 3 tentativas." << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    ConnectionManager::~ConnectionManager()
    {
        std::cout << "Liberando conexoes..." << std::endl;
        for(Connection* connection : connections)
        {
            delete connection;
        }
        connections.clear();
    }

    void ConnectionManager::onDataReceived(const uint8_t* data, size_t length, const Network::Endpoint& peerEndpoint)
    {
        Connection* connection = findConnection(peerEndpoint);
        if(connection == nullptr)
        {
            connection = createConnection(peerEndpoint);
        }
        connection->handleDataReceived(data, length);
    }

    void ConnectionManager::setOnDataReceived(DataCallback callback)
    {
        dataCallback = std::move(callback);
    }

    void ConnectionManager::run()
    {
        static constexpr std::chrono::milliseconds POLL_INTERVAL(100);
        uint8_t buffer[2048];

        while (true)
        {
            if (udpSocket4.waitForData(POLL_INTERVAL))
            {
                size_t receivedLength;
                Network::Endpoint srcEndpoint;

                if (udpSocket4.recvFrom(buffer, sizeof(buffer), receivedLength, srcEndpoint))
                    onDataReceived(buffer, receivedLength, srcEndpoint);
            }

            for (Connection* connection : connections)
                connection->update();

            connections.erase(
                std::remove_if(connections.begin(), connections.end(), [](Connection* connection)
                {
                    if (connection->getState() != CLOSED)
                        return false;

                    delete connection;
                    return true;
                }),
                connections.end()
            );
        }
    }

    Connection* ConnectionManager::findConnection(const Network::Endpoint& peerEndpoint)
    {
        bool isPeerIpv6 = peerEndpoint.isIpv6;
        uint16_t peerPort = peerEndpoint.port;
        const uint8_t* peerIp = peerEndpoint.ip;

        Network::Endpoint connEndpoint;
        bool isConnIpv6;
        uint16_t connPort;
        const uint8_t* connIp;

        for(Connection* connection : connections)
        {   
            connEndpoint = connection->getPeerEndpoint();
            isConnIpv6 = connEndpoint.isIpv6;
            connPort = connEndpoint.port;
            connIp = connEndpoint.ip;

            if(isPeerIpv6 == isConnIpv6 &&
               peerPort == connPort &&
               memcmp(peerIp, connIp, isPeerIpv6 ? 16 : 4) == 0)
            {
                return connection;
            }
        }
        return nullptr;
    }
    
    Connection* ConnectionManager::createConnection(const Network::Endpoint& peerEndpoint)
    {
        Connection* newConnection = new Connection(peerEndpoint, peerEndpoint.isIpv6 ? udpSocket6 : udpSocket4);

        if (dataCallback)
        {
            newConnection->setOnDataReceived([this, newConnection](const std::vector<uint8_t>& data)
            {
                dataCallback(*newConnection, data);
            });
        }

        connections.push_back(newConnection);
        std::cout << "Nova conexao criada: " << (peerEndpoint.isIpv6 ? "IPv6" : "IPv4") << " - " << Utils::IpFormatter::formatIp(peerEndpoint.ip, peerEndpoint.isIpv6) << ":" << peerEndpoint.port << std::endl;
        return newConnection;
    }
}