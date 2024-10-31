#pragma once
#include <string>
#include <array>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>

namespace Hashing
{
    class IHasher
    {
    public:
        virtual ~IHasher() = default;
        virtual std::string hash(const std::string& msg) const = 0;
    };

    class Md5 : public IHasher
    {
    public:
        std::string hash(const std::string& msg) const override
        {
            uint32_t a0 = 0x67452301; // A
            uint32_t b0 = 0xefcdab89; // B
            uint32_t c0 = 0x98badcfe; // C
            uint32_t d0 = 0x10325476; // D

            static const std::array<int, 64> S = {
                7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
                4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
            };

            static const std::array<uint32_t, 64> K = {
                0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
                0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
                0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
                0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
                0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
                0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
                0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
                0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
                0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
                0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
                0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
                0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
                0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
                0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
                0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
                0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
            };

            // Append "1" bit to message
            std::string processed = msg;
            processed += static_cast<char>(0x80);

            // Padding with zeros
            size_t original_length = processed.size() * 8; // in bits
            while ((processed.size() * 8) % 512 != 448)
                processed += static_cast<char>(0x00);

            // Append original length in bits mod 2^64 to message
            uint64_t original_length_bits = original_length;
            for (int i = 0; i < 8; ++i)
            {
                processed += static_cast<char>((original_length_bits >> (i * 8)) & 0xFF);
            }

            // Process the message in successive 512-bit chunks:
            for (size_t chunk = 0; chunk < processed.size(); chunk += 64)
            {
                // Break chunk into 16 32-bit little-endian words M[j], 0 ≤ j ≤ 15
                uint32_t M[16];
                for (int i = 0; i < 16; ++i)
                {
                    M[i] = static_cast<uint8_t>(processed[chunk + i * 4]) |
                           (static_cast<uint8_t>(processed[chunk + i * 4 + 1]) << 8) |
                           (static_cast<uint8_t>(processed[chunk + i * 4 + 2]) << 16) |
                           (static_cast<uint8_t>(processed[chunk + i * 4 + 3]) << 24);
                }

                // Initialize hash value for this chunk:
                uint32_t A = a0;
                uint32_t B = b0;
                uint32_t C = c0;
                uint32_t D = d0;

                // Main loop:
                for (int i = 0; i < 64; ++i)
                {
                    uint32_t F, g;

                    if (i < 16)
                    {
                        F = (B & C) | ((~B) & D);
                        g = i;
                    }
                    else if (i < 32)
                    {
                        F = (D & B) | ((~D) & C);
                        g = (5 * i + 1) % 16;
                    }
                    else if (i < 48)
                    {
                        F = B ^ C ^ D;
                        g = (3 * i + 5) % 16;
                    }
                    else
                    {
                        F = C ^ (B | (~D));
                        g = (7 * i) % 16;
                    }

                    uint32_t temp = D;
                    D = C;
                    C = B;
                    B = B + left_rotate((A + F + K[i] + M[g]), S[i]);
                    A = temp;
                }

                // Add this chunk's hash to result:
                a0 += A;
                b0 += B;
                c0 += C;
                d0 += D;
            }

            // final hash value
            std::ostringstream oss;
            oss << std::hex << std::setfill('0');
            oss << std::setw(8) << a0;
            oss << std::setw(8) << b0;
            oss << std::setw(8) << c0;
            oss << std::setw(8) << d0;
            return oss.str();
        }

    private:
        // Left-rotate a 32-bit integer x by n bits
        static uint32_t left_rotate(uint32_t x, int n)
        {
            return (x << n) | (x >> (32 - n));
        }
    };
}
