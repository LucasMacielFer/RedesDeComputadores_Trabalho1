#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <optional>
#include <zlib.h>
#include "protocol/protocol.h"

namespace Protocol
{
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