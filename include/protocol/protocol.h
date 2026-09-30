namespace Protocol
{
    enum Flags : uint8_t
    {
        SYN = 1 << 0,
        ACK = 1 << 1,
        NACK = 1 << 3,
        FIN = 1 << 2
    };

    enum ResultCode : uint8_t
    {
        SUCCESS = 0,
        FILE_NOT_FOUND = 1,
        PERMISSION_DENIED = 2
    };

    struct Header
    {
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
}