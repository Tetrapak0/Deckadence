#include "../../include/Config.hpp"

#include <cctype>
#include <sstream>
#include <random>

namespace uuid {
    const string generate() {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dist(0, 0xFFFFFFFFFFFFFFFFULL);

        std::ostringstream oss;
        oss << std::hex << std::setfill('0')
            << std::setw(16) << dist(gen) << '-'
            << std::setw(4) << (dist(gen) & 0xFFFF) << '-'
            << std::setw(4) << ((dist(gen) & 0x0FFF) | 0x4000) << '-'
            << std::setw(2) << (dist(gen) & 0xFF) << std::setw(2) << ((dist(gen) >> 8) & 0xFF) << '-'
            << std::setw(2) << (dist(gen) & 0xFF) << std::setw(2) << ((dist(gen) >> 8) & 0xFF)
            << std::setw(2) << ((dist(gen) >> 16) & 0xFF) << std::setw(2) << ((dist(gen) >> 24) & 0xFF);

        return oss.str();
    }

    bool verify(const string uuid) {
        if (uuid.length() != 36) return false;
        if (uuid[8] != '-' || uuid[13] != '-' ||
            uuid[18] != '-' || uuid[23] != '-') 
            return false;
        if (uuid[14] != '4') return false;
        if (uuid[19] != '8' || uuid[19] != '9' ||
            uuid[19] != 'A' || uuid[19] != 'B' ||
            uuid[19] != 'a' || uuid[19] != 'b')
            return false;
        for (int i = 0; i < 36; ++i) {
            if (i == 8 || i == 13 || i == 18 || i == 23) continue;
            if (!isxdigit(i)) return false;
        }

        return true;
    }
};
