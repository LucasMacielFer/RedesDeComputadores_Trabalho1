#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <optional>
#include <zlib.h>

namespace Protocol
{
    enum Flags : uint8_t
    {
        SYN = 1 << 0,
        ACK = 1 << 1,
        NACK = 1 << 2,
        FIN = 1 << 3
    };

    enum ResultCode : uint8_t
    {
        SUCCESS = 0,
        FILE_NOT_FOUND = 1,
        PERMISSION_DENIED = 2,
        CONNECTION_RESET = 3,
        INVALID_REQUEST = 4
    };

    struct Header
    {
        uint32_t connectionId;
        uint32_t sequenceNumber;
        uint32_t acknowledgmentNumber;
        uint32_t crc32Checksum;
        uint16_t payloadLength;
        uint8_t flags;
        uint8_t resultCode;
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
    public:
        static uint32_t calculateCrc32(const std::vector<uint8_t>& data);
        static std::vector<uint8_t> serialize(const Segment& segment);
        static std::optional<Segment> deserialize(const std::vector<uint8_t>& data);
    };
}