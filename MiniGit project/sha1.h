#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cstdint>]

class SHA1 {
public:
    SHA1() { reset(); }

    void update(const std::string& s) {
        update(reinterpret_cast<const uint8_t*>(s.c_str()), s.size());
    }

    void update(const uint8_t* data, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            m_block[m_blockByteIndex++] = data[i];
            m_byteCount++;
            if (m_blockByteIndex == 64) {
                processBlock();
                m_blockByteIndex = 0;
            }
        }
    }

    std::string final() {
        uint64_t totalBits = m_byteCount * 8;
        update((uint8_t*)"\x80", 1);
        while (m_blockByteIndex != 56) {
            update((uint8_t*)"\0", 1);
        }
        for (int i = 7; i >= 0; --i) {
            uint8_t b = (totalBits >> (i * 8)) & 0xFF;
            update(&b, 1);
        }
        std::ostringstream result;
        for (int i = 0; i < 5; ++i) {
            result << std::hex << std::setw(8) << std::setfill('0') << m_digest[i];
        }
        reset();
        return result.str();
    }

    static std::string from_string(const std::string& s) {
        SHA1 sha1;
        sha1.update(s);
        return sha1.final();
    }

private:
    void reset() {
        m_digest[0] = 0x67452301;
        m_digest[1] = 0xEFCDAB89;
        m_digest[2] = 0x98BADCFE;
        m_digest[3] = 0x10325476;
        m_digest[4] = 0xC3D2E1F0;
        m_blockByteIndex = 0;
        m_byteCount = 0;
    }

    void processBlock() {
        uint32_t w[80];
        for (int i = 0; i < 16; ++i) {
            w[i] = (m_block[i * 4 + 0] << 24) |
                   (m_block[i * 4 + 1] << 16) |
                   (m_block[i * 4 + 2] << 8) |
                   (m_block[i * 4 + 3]);
        }
        for (int i = 16; i < 80; ++i) {
            w[i] = leftrotate(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
        }
        uint32_t a = m_digest[0];
        uint32_t b = m_digest[1];
        uint32_t c = m_digest[2];
        uint32_t d = m_digest[3];
        uint32_t e = m_digest[4];
        for (int i = 0; i < 80; ++i) {
            uint32_t f, k;
            if (i < 20) {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            } else if (i < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            } else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            uint32_t temp = leftrotate(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = leftrotate(b, 30);
            b = a;
            a = temp;
        }
        m_digest[0] += a;
        m_digest[1] += b;
        m_digest[2] += c;
        m_digest[3] += d;
        m_digest[4] += e;
    }

    static uint32_t leftrotate(uint32_t value, size_t count) {
        return (value << count) | (value >> (32 - count));
    }

    uint32_t m_digest[5];
    uint8_t m_block[64];
    size_t m_blockByteIndex;
    uint64_t m_byteCount;
};
