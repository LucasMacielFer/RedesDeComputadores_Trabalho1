#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <zlib.h>

namespace Protocol
{
    enum Flags : uint16_t
    {
        SYN = 1 << 0,
        ACK = 1 << 1,
        NACK = 1 << 2,
        FIN = 1 << 3
    };

    struct Header
    {
        uint32_t connectionId;
        uint32_t sequenceNumber;
        uint32_t acknowledgmentNumber;
        uint32_t crc32Checksum;
        uint16_t payloadLength;
        uint16_t flags;
    };

    struct Segment
    {
        Header segmentHeader;
        std::vector<uint8_t> payload;
    };

    class Serializer
    {
    private:
        static const size_t HEADER_SIZE = sizeof(Header);
        static const size_t MAX_PAYLOAD_SIZE = 1200;

        static uint32_t calculateCrc32(const std::vector<uint8_t>& data);
    public:
        static std::vector<uint8_t> serialize(const Segment& segment);
        static Segment deserialize(const std::vector<uint8_t>& data);
    };
}