#include "protocol/serializer.h"

namespace Protocol
{
    uint32_t Serializer::calculateCrc32(const std::vector<uint8_t>& data)
    {
        return crc32(0L, data.data(), data.size());
    }

    std::vector<uint8_t> Serializer::serialize(const Segment& segment)
    {
        if (segment.payload.size() > MAX_PAYLOAD_SIZE)
        {
            std::cerr << "ERRO: Tamanho do payload excede o limite máximo de " << MAX_PAYLOAD_SIZE << " bytes." << std::endl;
        }

        std::vector<uint8_t> serializedData(HEADER_SIZE + segment.segmentHeader.payloadLength);
        memcpy(serializedData.data(), &segment.segmentHeader, HEADER_SIZE);
        memcpy(serializedData.data() + HEADER_SIZE, segment.payload.data(), segment.segmentHeader.payloadLength);
        return serializedData;
    }

    Segment Serializer::deserialize(const std::vector<uint8_t>& data)
    {
        if (data.size() < HEADER_SIZE)
        {
            std::cerr << "ERRO: Dados insuficientes para desserializar o segmento." << std::endl;
        }

        Segment segment;
        memcpy(&segment.segmentHeader, data.data(), HEADER_SIZE);
        segment.payload.resize(segment.segmentHeader.payloadLength);
        memcpy(segment.payload.data(), data.data() + HEADER_SIZE, segment.segmentHeader.payloadLength);

        return segment;
    }
}