#pragma once
#include <cstdint>
#include <vector>

namespace Protocol
{
    enum Flags : uint16_t
    {
        SYN = 1 << 0,
        ACK = 1 << 1,
        NACK = 1 << 2,
        FIN = 1 << 3
    };

    enum ApplicationCodes : uint8_t
    {
        REQUEST = 0x01,
        METADATA = 0x02,
        DATA = 0x03,
        FILE_ERROR = 0x04
    };

    enum FileErrorReason : uint8_t
    {
        NOT_FOUND = 1,
        PERMISSION_DENIED = 2
    };

    struct Header
    {
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
}