#include <sstream>
#include <iomanip>
#include <cinttypes>

namespace Utils
{
    class IpFormatter
    {
    public:
        static std::string formatIp(const uint8_t* bytes, bool isIpv6) {
            std::ostringstream oss;
            if (!isIpv6) {
                for (int i = 0; i < 4; ++i) {
                    if (i > 0) oss << ".";
                    oss << static_cast<int>(bytes[i]);
                }
            } else {
                oss << std::hex << std::setfill('0');
                for (int i = 0; i < 16; i += 2) {
                    if (i > 0) oss << ":";
                    oss << std::setw(2) << static_cast<int>(bytes[i])
                        << std::setw(2) << static_cast<int>(bytes[i + 1]);
                }
            }
            return oss.str();
        }
    };
}