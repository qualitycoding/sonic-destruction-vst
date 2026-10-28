#include "dsp/Tables.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace faultcounter
{
namespace
{
uint32_t rotateRight (uint32_t value, uint32_t count)
{
    return (value >> count) | (value << (32u - count));
}

std::string sha256 (const std::vector<uint8_t>& input)
{
    static constexpr std::array<uint32_t, 64> k {{
        0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
        0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
        0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
        0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
        0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
        0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
        0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
        0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
    }};

    std::vector<uint8_t> message = input;
    const uint64_t bitLength = uint64_t (message.size()) * 8u;
    message.push_back (0x80u);
    while ((message.size() % 64u) != 56u)
        message.push_back (0u);
    for (int shift = 56; shift >= 0; shift -= 8)
        message.push_back (uint8_t ((bitLength >> uint32_t (shift)) & 0xffu));

    std::array<uint32_t, 8> state {{
        0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
        0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u
    }};

    std::array<uint32_t, 64> schedule {};
    for (size_t offset = 0; offset < message.size(); offset += 64)
    {
        for (size_t i = 0; i < 16; ++i)
        {
            const size_t index = offset + i * 4;
            schedule[i] = (uint32_t (message[index]) << 24u)
                          | (uint32_t (message[index + 1]) << 16u)
                          | (uint32_t (message[index + 2]) << 8u)
                          | uint32_t (message[index + 3]);
        }

        for (size_t i = 16; i < 64; ++i)
        {
            const uint32_t s0 = rotateRight (schedule[i - 15], 7) ^ rotateRight (schedule[i - 15], 18) ^ (schedule[i - 15] >> 3u);
            const uint32_t s1 = rotateRight (schedule[i - 2], 17) ^ rotateRight (schedule[i - 2], 19) ^ (schedule[i - 2] >> 10u);
            schedule[i] = schedule[i - 16] + s0 + schedule[i - 7] + s1;
        }

        uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
        uint32_t e = state[4], f = state[5], g = state[6], h = state[7];

        for (size_t i = 0; i < 64; ++i)
        {
            const uint32_t s1 = rotateRight (e, 6) ^ rotateRight (e, 11) ^ rotateRight (e, 25);
            const uint32_t choose = (e & f) ^ ((~e) & g);
            const uint32_t temp1 = h + s1 + choose + k[i] + schedule[i];
            const uint32_t s0 = rotateRight (a, 2) ^ rotateRight (a, 13) ^ rotateRight (a, 22);
            const uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
            const uint32_t temp2 = s0 + majority;
            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
        state[5] += f;
        state[6] += g;
        state[7] += h;
    }

    std::ostringstream hex;
    hex << std::hex << std::setfill ('0');
    for (const auto word : state)
        hex << std::setw (8) << word;
    return hex.str();
}
} // namespace

Tables Tables::loadProcedural (const std::filesystem::path& path)
{
    std::ifstream input (path, std::ios::binary);
    if (! input)
        throw std::runtime_error ("unable to open procedural table asset");

    Tables result;
    result.data_.assign (std::istreambuf_iterator<char> (input), std::istreambuf_iterator<char>());
    if (result.data_.size() != tableCount * tableRowLength)
        throw std::runtime_error ("procedural table asset has the wrong size");
    return result;
}

std::string Tables::sha256File (const std::filesystem::path& path)
{
    std::ifstream input (path, std::ios::binary);
    if (! input)
        throw std::runtime_error ("unable to open file for SHA-256");

    std::vector<uint8_t> bytes ((std::istreambuf_iterator<char> (input)), std::istreambuf_iterator<char>());
    return sha256 (bytes);
}

uint8_t Tables::lookup (int table, uint8_t code) const noexcept
{
    const int safeTable = std::clamp (table, 0, tableCount - 1);
    return data_[size_t (safeTable) * tableRowLength + code];
}

int Tables::size() const noexcept
{
    return int (data_.size() / tableRowLength);
}

const std::vector<uint8_t>& Tables::data() const noexcept
{
    return data_;
}
} // namespace faultcounter
