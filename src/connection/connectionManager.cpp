#include "connection/connectionManager.h"

namespace Connection
{
    ConnectionManager::ConnectionManager(uint16_t port):
        udpSocket4(false),
        udpSocket6(true)
    {
        connections = std::vector<Connection*>();

        udpSocket4.bind(port);
        if(!udpSocket4.isBound())
        {
            for(int i = 0; i < 3; ++i)
            {
                std::cerr << "ERRO: Falha ao vincular o socket UDP IPv4 a porta " << port << ". Tentativa " << (i + 1) << " de 3." << std::endl;
                udpSocket4.bind(port);

                if(udpSocket4.isBound())
                    break;
            }
        }

        if(!udpSocket4.isBound())
        {
            std::cerr << "ERRO: Falha ao vincular o socket UDP IPv4 a porta " << port << " apos 3 tentativas." << std::endl;
            exit(EXIT_FAILURE);
        }

        udpSocket6.bind(port);
        if(!udpSocket6.isBound())
        {
            for(int i = 0; i < 3; ++i)
            {
                std::cerr << "ERRO: Falha ao vincular o socket UDP IPv6 a porta " << port << ". Tentativa " << (i + 1) << " de 3." << std::endl;
                udpSocket6.bind(port);

                if(udpSocket6.isBound())
                    break;
            }
        }

        if(!udpSocket6.isBound())
        {
            std::cerr << "ERRO: Falha ao vincular o socket UDP IPv6 a porta " << port << " apos 3 tentativas." << std::endl;
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

    void ConnectionManager::onDataReceived(const uint8_t* data, size_t length, const uint8_t* peerIp, uint16_t peerPort)
    {
        std::vector<uint8_t> dataVector(data, data + length);
        std::optional<Protocol::Segment> segmentOpt = Protocol::Serializer::deserialize(dataVector);

        if(segmentOpt != std::nullopt)
        {
            Protocol::Segment segment = segmentOpt.value();
            Connection* connection = findConnection(peerIp, peerPort, false, segment.segmentHeader.connectionId);

            if(connection != nullptr)
            {
                connection->handleSegment(segment);
            }
            else
            {
                connection = createConnection(peerIp, peerPort, false, segment.segmentHeader.connectionId);
                connection->handleSegment(segment);
            }
        }
        else
        {
            std::cerr << "ERRO: Falha ao desserializar o segmento recebido." << std::endl;
        }
    }

    Connection* ConnectionManager::findConnection(const uint8_t* peerIp, uint16_t peerPort, bool isIpv6, uint32_t connectionId)
    {
        for(Connection* connection : connections)
        {
            if(connection->getPeerPort() == peerPort &&
               memcmp(connection->getPeerIp(), peerIp, isIpv6 ? 16 : 4) == 0 &&
               connection->isPeerIpv6() == isIpv6 &&
               connection->getConnectionId() == connectionId)
            {
                return connection;
            }
        }
        return nullptr;
    }
    
    Connection* ConnectionManager::createConnection(const uint8_t* peerIp, uint16_t peerPort, bool isIpv6, uint32_t connectionId)
    {
        Connection* newConnection = new Connection(peerIp, peerPort, isIpv6);
        newConnection->setConnectionId(connectionId);
        connections.push_back(newConnection);
        std::cout << "Nova conexao criada: " << (isIpv6 ? "IPv6" : "IPv4") << " - " << Utils::IpFormatter::formatIp(peerIp, isIpv6) << ":" << peerPort << std::endl;
        return newConnection;
    }
}