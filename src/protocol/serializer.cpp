#include "protocol/serializer.h"

namespace Protocol
{
    uint32_t Serializer::calculateCrc32(const std::vector<uint8_t>& data)
    {
        return crc32(0L, data.data(), data.size());
    }

    std::vector<uint8_t> Serializer::serialize(const Segment& segment)
    {
        if(segment.payload.size() > MAX_PAYLOAD_SIZE)
        {
            throw std::invalid_argument("ERRO: Tamanho do payload excede o limite máximo de " + std::to_string(MAX_PAYLOAD_SIZE) + " bytes.");
        }
        if(segment.payload.size() != segment.segmentHeader.payloadLength)
        {
            throw std::invalid_argument("ERRO: Tamanho do payload diferente do tamanho declarado no header.");
        }

        std::vector<uint8_t> serializedData(HEADER_SIZE + segment.segmentHeader.payloadLength);        
        memcpy(serializedData.data(), &segment.segmentHeader, HEADER_SIZE);
        if(segment.segmentHeader.payloadLength > 0)
        {
            memcpy(serializedData.data() + HEADER_SIZE, segment.payload.data(), segment.segmentHeader.payloadLength);
        }
        
        return serializedData;
    }

    Segment Serializer::deserialize(const std::vector<uint8_t>& data, bool& success)
    {
        success = false;

        if (data.size() < HEADER_SIZE)
        {
            std::cerr << "ERRO: Dados insuficientes para desserializar o segmento." << std::endl;
            return Segment();
        }

        Segment segment;
        memcpy(&segment.segmentHeader, data.data(), HEADER_SIZE);
        segment.payload.resize(segment.segmentHeader.payloadLength);
        memcpy(segment.payload.data(), data.data() + HEADER_SIZE, segment.segmentHeader.payloadLength);

        success = true;
        return segment;
    }
}