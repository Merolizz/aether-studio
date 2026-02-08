#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>

namespace aether {

class SHA256 {
public:
    SHA256() {
        m_datalen = 0;
        m_bitlen = 0;
        m_state[0] = 0x6a09e667;
        m_state[1] = 0xbb67ae85;
        m_state[2] = 0x3c6ef372;
        m_state[3] = 0xa54ff53a;
        m_state[4] = 0x510e527f;
        m_state[5] = 0x9b05688c;
        m_state[6] = 0x1f83d9ab;
        m_state[7] = 0x5be0cd19;
    }
    
    void update(const uint8_t* data, size_t len);
    void final(uint8_t* hash);
    
    static std::string hash(const std::string& input);

private:
    void transform(const uint8_t* data);
    
    uint8_t m_data[64];
    uint32_t m_datalen;
    uint64_t m_bitlen;
    uint32_t m_state[8];
};

} // namespace aether
