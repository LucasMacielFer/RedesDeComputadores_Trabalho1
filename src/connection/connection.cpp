#include "connection/connection.h"
#include "utils/ipFormatter.h"
#include <algorithm>

namespace Connection
{
    namespace
    {
        std::string peerLabel(const Network::Endpoint& endpoint)
        {
            return Utils::IpFormatter::formatIp(endpoint.ip, endpoint.isIpv6) + ":" + std::to_string(endpoint.port);
        }

        bool trySerialize(const Protocol::Segment& segment, std::vector<uint8_t>& out)
        {
            try
            {
                out = Protocol::Serializer::serialize(segment);
                return true;
            }
            catch (const std::exception& e)
            {
                std::cout << "[TX FAIL] " << e.what() << std::endl;
                return false;
            }
        }
    }

    Connection::Connection(const Network::Endpoint peerEndpoint, Network::UdpSocket& udpSocket) :
        udpSocket(udpSocket),
        peerEndpoint(peerEndpoint),
        state(CLOSED),
        nextSendSequence(0),
        nextReceiveSequence(0),
        waitingAck(false),
        pendingSequence(0),
        retryCount(0),
        packetWasRetransmitted(false),
        timerRunning(false),
        hasRttSample(false),
        srtt(0),
        rttvar(0),
        rto(INITIAL_RTO)
    {
        std::cout << "[CONN] Conexao criada com " << peerLabel(peerEndpoint) << std::endl;
    }

    Connection::~Connection()
    {
        std::cout << "[CONN] Conexao encerrada com " << peerLabel(peerEndpoint) << std::endl;
    }

    const Network::Endpoint& Connection::getPeerEndpoint() const
    {
        return peerEndpoint;
    }

    ConnectionState Connection::getState() const
    {
        return state;
    }

    bool Connection::isIdle() const
    {
        return state == ESTABLISHED && !waitingAck;
    }

    void Connection::setOnDataReceived(DataCallback callback)
    {
        dataReceivedCallback = std::move(callback);
    }

    void Connection::setOnIdle(IdleCallback callback)
    {
        idleCallback = std::move(callback);
    }

    bool Connection::connect()
    {
        if (state != CLOSED)
        {
            std::cout << "[CONN] Ignorado: conexao com " << peerLabel(peerEndpoint) << " nao esta CLOSED." << std::endl;
            return false;
        }

        Protocol::Segment segment{};
        segment.segmentHeader.sequenceNumber = nextSendSequence;
        segment.segmentHeader.acknowledgmentNumber = 0;
        segment.segmentHeader.flags = Protocol::SYN;
        segment.segmentHeader.resultCode = Protocol::SUCCESS;
        segment.segmentHeader.payloadLength = 0;
        segment.payload = {};
        segment.segmentHeader.crc32Checksum = Protocol::Serializer::calculateCrc32(segment.payload);

        std::vector<uint8_t> packet;
        if (!trySerialize(segment, packet))
            return false;

        if (!udpSocket.sendTo(packet.data(), packet.size(), peerEndpoint))
        {
            std::cout << "[TX FAIL] Falha ao enviar SYN para " << peerLabel(peerEndpoint) << std::endl;
            return false;
        }

        pendingDataPacket = packet;
        pendingSequence = nextSendSequence;
        waitingAck = true;
        retryCount = 0;
        packetWasRetransmitted = false;
        startTimer();

        state = SYN_SENT;
        std::cout << "[TX] SYN " << nextSendSequence << " -> " << peerLabel(peerEndpoint) << std::endl;
        return true;
    }

    void Connection::handleDataReceived(const uint8_t* data, size_t length)
    {
        bool success;
        Protocol::Segment segment = Protocol::Serializer::deserialize(std::vector<uint8_t>(data, data + length), success);

        if (!success)
        {
            std::cout << "[RX FAIL] Falha ao desserializar segmento de " << peerLabel(peerEndpoint) << std::endl;
            return;
        }

        handleSegment(segment);
    }

    bool Connection::sendData(const uint8_t* data, size_t length)
    {
        if (state != ESTABLISHED)
        {
            std::cout << "[TX FAIL] Conexao nao esta ESTABLISHED para " << peerLabel(peerEndpoint) << std::endl;
            return false;
        }

        if (waitingAck)
        {
            std::cout << "[TX FAIL] Aguardando ACK do segmento anterior para " << peerLabel(peerEndpoint) << std::endl;
            return false;
        }

        Protocol::Segment segment{};
        segment.payload.assign(data, data + length);
        segment.segmentHeader.sequenceNumber = nextSendSequence;
        segment.segmentHeader.acknowledgmentNumber = 0;
        segment.segmentHeader.flags = 0;
        segment.segmentHeader.resultCode = Protocol::SUCCESS;
        segment.segmentHeader.payloadLength = static_cast<uint16_t>(length);
        segment.segmentHeader.crc32Checksum = Protocol::Serializer::calculateCrc32(segment.payload);

        std::vector<uint8_t> packet;
        if (!trySerialize(segment, packet))
            return false;

        if (!udpSocket.sendTo(packet.data(), packet.size(), peerEndpoint))
        {
            std::cout << "[TX FAIL] Falha ao enviar SEQ " << nextSendSequence << " para " << peerLabel(peerEndpoint) << std::endl;
            return false;
        }

        pendingDataPacket = packet;
        pendingSequence = nextSendSequence;
        waitingAck = true;
        retryCount = 0;
        packetWasRetransmitted = false;
        startTimer();

        std::cout << "[TX] SEQ " << nextSendSequence << " (" << length << " bytes) -> " << peerLabel(peerEndpoint) << std::endl;
        return true;
    }

    bool Connection::close()
    {
        if (state != ESTABLISHED)
        {
            std::cout << "[CONN] Close ignorado: conexao com " << peerLabel(peerEndpoint) << " nao esta ESTABLISHED." << std::endl;
            return false;
        }

        if (waitingAck)
        {
            std::cout << "[CONN] Close ignorado: ha um segmento pendente de ACK para " << peerLabel(peerEndpoint) << std::endl;
            return false;
        }

        Protocol::Segment segment{};
        segment.segmentHeader.sequenceNumber = nextSendSequence;
        segment.segmentHeader.acknowledgmentNumber = 0;
        segment.segmentHeader.flags = Protocol::FIN;
        segment.segmentHeader.resultCode = Protocol::SUCCESS;
        segment.segmentHeader.payloadLength = 0;
        segment.payload = {};
        segment.segmentHeader.crc32Checksum = Protocol::Serializer::calculateCrc32(segment.payload);

        std::vector<uint8_t> packet;
        if (!trySerialize(segment, packet))
            return false;

        if (!udpSocket.sendTo(packet.data(), packet.size(), peerEndpoint))
        {
            std::cout << "[TX FAIL] Falha ao enviar FIN para " << peerLabel(peerEndpoint) << std::endl;
            return false;
        }

        pendingDataPacket = packet;
        pendingSequence = nextSendSequence;
        waitingAck = true;
        retryCount = 0;
        packetWasRetransmitted = false;
        startTimer();

        state = CLOSING;
        std::cout << "[TX] FIN " << nextSendSequence << " -> " << peerLabel(peerEndpoint) << std::endl;
        return true;
    }

    void Connection::update()
    {
        if (!waitingAck || !timerRunning)
            return;

        if (timeoutExpired())
            retransmitPending();
    }

    void Connection::handleSegment(const Protocol::Segment& segment)
    {
        const Protocol::Header& header = segment.segmentHeader;

        if (header.resultCode != Protocol::SUCCESS)
        {
            std::cout << "[REQUEST FAILED] " << (header.resultCode == Protocol::FILE_NOT_FOUND ? "Arquivo nao encontrado" : "Permissao negada")
                       << " (" << peerLabel(peerEndpoint) << ")" << std::endl;

            waitingAck = false;
            pendingDataPacket.clear();
            stopTimer();
            packetWasRetransmitted = false;
            retryCount = 0;
            state = CLOSED;
            return;
        }

        const bool ack = (header.flags & Protocol::ACK) != 0;
        const bool nack = (header.flags & Protocol::NACK) != 0;
        const bool fin = (header.flags & Protocol::FIN) != 0;
        const bool syn = (header.flags & Protocol::SYN) != 0;

        if (ack)
            handleAck(header);
        else if (nack)
            handleNack(header);
        else if (fin)
            handleFin(segment);
        else if (syn)
            handleSyn(segment);
        else
            handleDataSegment(segment);
    }

    void Connection::handleAck(const Protocol::Header& header)
    {
        if (!waitingAck || header.acknowledgmentNumber != pendingSequence)
            return;

        if (!packetWasRetransmitted)
        {
            const auto rtt = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timerStart);
            updateRto(rtt);
        }

        waitingAck = false;
        pendingDataPacket.clear();
        stopTimer();
        packetWasRetransmitted = false;
        retryCount = 0;
        ++nextSendSequence;

        if (state == SYN_SENT)
        {
            state = ESTABLISHED;
            std::cout << "[RX] SYN ACK " << header.acknowledgmentNumber << " <- " << peerLabel(peerEndpoint) << std::endl;
        }
        else if (state == CLOSING)
        {
            state = CLOSED;
            std::cout << "[CONN] Conexao encerrada (FIN confirmado) com " << peerLabel(peerEndpoint) << std::endl;
        }
        else
        {
            std::cout << "[RX] ACK " << header.acknowledgmentNumber << " <- " << peerLabel(peerEndpoint) << std::endl;

            if (idleCallback)
            {
                IdleCallback callback = std::move(idleCallback);
                idleCallback = nullptr;
                callback();
            }
        }
    }

    void Connection::handleNack(const Protocol::Header& header)
    {
        if (!waitingAck || header.acknowledgmentNumber != pendingSequence)
            return;

        std::cout << "[RX] NACK " << header.acknowledgmentNumber << " <- " << peerLabel(peerEndpoint) << ", retransmitindo." << std::endl;
        retransmitPending();
    }

    void Connection::handleDataSegment(const Protocol::Segment& segment)
    {
        const Protocol::Header& header = segment.segmentHeader;

        if (state != ESTABLISHED && state != SYN_RECEIVED)
            return;

        if (header.sequenceNumber == nextReceiveSequence)
        {
            const uint32_t computedCrc = Protocol::Serializer::calculateCrc32(segment.payload);
            if (computedCrc != header.crc32Checksum)
            {
                std::cout << "[RX FAIL] Payload corrompido (SEQ " << header.sequenceNumber << ") de " << peerLabel(peerEndpoint) << std::endl;
                sendNack(header.sequenceNumber);
                return;
            }

            if (state == SYN_RECEIVED)
            {
                state = ESTABLISHED;
                std::cout << "[RX] SEQ " << header.sequenceNumber << " <- " << peerLabel(peerEndpoint) << " (CONEXAO ESTABELECIDA)." << std::endl;
            }

            std::cout << "[RX] SEQ " << header.sequenceNumber << " (" << segment.payload.size() << " bytes) <- " << peerLabel(peerEndpoint) << std::endl;

            ++nextReceiveSequence;
            sendAck(header.sequenceNumber);

            if (dataReceivedCallback)
                dataReceivedCallback(segment.payload);

            return;
        }

        if (header.sequenceNumber < nextReceiveSequence)
        {
            std::cout << "[RX] SEQ " << header.sequenceNumber << " <- " << peerLabel(peerEndpoint) << "(DUPLICADO)" << std::endl;
            sendAck(header.sequenceNumber);
        }
    }

    void Connection::handleSyn(const Protocol::Segment& segment)
    {
        const Protocol::Header& header = segment.segmentHeader;

        if (state == CLOSED)
        {
            nextReceiveSequence = header.sequenceNumber + 1;
            state = SYN_RECEIVED;
            std::cout << "[RX] SYN " << header.sequenceNumber << " <- " << peerLabel(peerEndpoint) << std::endl;
        }
        else if (state == SYN_RECEIVED && header.sequenceNumber + 1 == nextReceiveSequence)
        {
            std::cout << "[RX] SYN " << header.sequenceNumber << " <- " << peerLabel(peerEndpoint) << " (DUPLICADO)" << std::endl;
        }
        else
        {
            return;
        }

        sendAck(header.sequenceNumber);
    }

    void Connection::handleFin(const Protocol::Segment& segment)
    {
        const Protocol::Header& header = segment.segmentHeader;

        if (state == CLOSED)
            return;

        if (header.sequenceNumber > nextReceiveSequence)
            return;

        if (header.sequenceNumber == nextReceiveSequence)
        {
            ++nextReceiveSequence;
            std::cout << "[RX] FIN " << header.sequenceNumber << " <- " << peerLabel(peerEndpoint) << std::endl;
        }
        else
        {
            std::cout << "[RX] FIN " << header.sequenceNumber << " <- " << peerLabel(peerEndpoint) << " (DUPLICADO)" << std::endl;
        }

        sendAck(header.sequenceNumber);

        if (state == CLOSING)
            return;

        state = CLOSED;
        std::cout << "[CONN] Conexao encerrada pelo peer " << peerLabel(peerEndpoint) << std::endl;
    }

    bool Connection::sendSegment(const Protocol::Segment& segment)
    {
        std::vector<uint8_t> packet;
        if (!trySerialize(segment, packet))
            return false;

        return udpSocket.sendTo(packet.data(), packet.size(), peerEndpoint);
    }

    bool Connection::sendAck(uint32_t sequence)
    {
        Protocol::Segment response{};
        response.segmentHeader.sequenceNumber = nextSendSequence;
        response.segmentHeader.acknowledgmentNumber = sequence;
        response.segmentHeader.flags = Protocol::ACK;
        response.segmentHeader.resultCode = Protocol::SUCCESS;
        response.segmentHeader.payloadLength = 0;
        response.payload = {};
        response.segmentHeader.crc32Checksum = Protocol::Serializer::calculateCrc32(response.payload);

        const bool ok = sendSegment(response);
        if (ok)
            std::cout << "[TX] ACK " << sequence << " -> " << peerLabel(peerEndpoint) << std::endl;
        else
            std::cout << "[TX FAIL] Falha ao enviar ACK " << sequence << " para " << peerLabel(peerEndpoint) << std::endl;

        return ok;
    }

    bool Connection::sendNack(uint32_t sequence)
    {
        Protocol::Segment response{};
        response.segmentHeader.sequenceNumber = nextSendSequence;
        response.segmentHeader.acknowledgmentNumber = sequence;
        response.segmentHeader.flags = Protocol::NACK;
        response.segmentHeader.resultCode = Protocol::SUCCESS;
        response.segmentHeader.payloadLength = 0;
        response.payload = {};
        response.segmentHeader.crc32Checksum = Protocol::Serializer::calculateCrc32(response.payload);

        const bool ok = sendSegment(response);
        if (ok)
            std::cout << "[TX] NACK " << sequence << " -> " << peerLabel(peerEndpoint) << std::endl;
        else
            std::cout << "[TX FAIL] Falha ao enviar NACK " << sequence << " para " << peerLabel(peerEndpoint) << std::endl;

        return ok;
    }

    void Connection::startTimer()
    {
        timerStart = std::chrono::steady_clock::now();
        timerRunning = true;
    }

    void Connection::stopTimer()
    {
        timerRunning = false;
    }

    bool Connection::timeoutExpired() const
    {
        if (!timerRunning)
            return false;

        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timerStart);
        return elapsed >= rto;
    }

    bool Connection::retransmitPending()
    {
        if (!waitingAck || pendingDataPacket.empty())
            return false;

        if (retryCount >= MAX_RETRIES)
        {
            giveUp();
            return false;
        }

        if (!udpSocket.sendTo(pendingDataPacket.data(), pendingDataPacket.size(), peerEndpoint))
        {
            std::cout << "[TX FAIL] Falha ao retransmitir SEQ " << pendingSequence << " para " << peerLabel(peerEndpoint) << std::endl;
            return false;
        }

        ++retryCount;
        packetWasRetransmitted = true;
        startTimer();

        std::cout << "[TX] Retransmissao " << retryCount << "/" << MAX_RETRIES << " do SEQ " << pendingSequence << " para " << peerLabel(peerEndpoint) << std::endl;
        return true;
    }

    void Connection::giveUp()
    {
        std::cout << "[CONN] Numero maximo de retransmissoes atingido para SEQ " << pendingSequence
                   << " (" << peerLabel(peerEndpoint) << "). Encerrando conexao." << std::endl;

        waitingAck = false;
        pendingDataPacket.clear();
        stopTimer();
        packetWasRetransmitted = false;
        retryCount = 0;

        state = CLOSED;
    }

    void Connection::updateRto(std::chrono::milliseconds rtt)
    {
        if (!hasRttSample)
        {
            srtt = rtt;
            rttvar = rtt / 2;
            hasRttSample = true;
        }
        else
        {
            const auto delta = (srtt > rtt) ? (srtt - rtt) : (rtt - srtt);
            rttvar = std::chrono::milliseconds(static_cast<long long>(0.75 * rttvar.count() + 0.25 * delta.count()));
            srtt = std::chrono::milliseconds(static_cast<long long>(0.875 * srtt.count() + 0.125 * rtt.count()));
        }

        const auto computedRto = srtt + std::max(std::chrono::milliseconds(1), 4 * rttvar);
        rto = std::clamp(computedRto, MIN_RTO, MAX_RTO);
    }
}
