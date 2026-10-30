// T-023: golden PCM and report files embed commit, environment-lock hash,
// parameter state, sample rate, block schedule, seed, and UTC generation
// time, and the embedded fixture hashes match the files on disk.
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace
{
// ---- Minimal JSON parser (sufficient for the generator-written manifest) ----
struct Json
{
    enum class Type { Null, Bool, Number, String, Array, Object };
    Type type = Type::Null;
    double number = 0.0;
    bool boolean = false;
    std::string string;
    std::vector<Json> array;
    std::map<std::string, Json> object;

    const Json& operator[] (const std::string& key) const
    {
        static const Json missing;
        const auto it = object.find (key);
        return it == object.end() ? missing : it->second;
    }
};

class JsonParser
{
public:
    explicit JsonParser (const std::string& text) : text_ (text) {}

    Json parse()
    {
        skipWs();
        return parseValue();
    }

private:
    const std::string& text_;
    size_t pos_ = 0;

    void skipWs()
    {
        while (pos_ < text_.size() && (text_[pos_] == ' ' || text_[pos_] == '\t' || text_[pos_] == '\n' || text_[pos_] == '\r'))
            ++pos_;
    }

    char peek() { return pos_ < text_.size() ? text_[pos_] : '\0'; }
    char take() { return pos_ < text_.size() ? text_[pos_++] : '\0'; }

    Json parseValue()
    {
        skipWs();
        Json value;
        switch (peek())
        {
            case '{':
            {
                take();
                value.type = Json::Type::Object;
                skipWs();
                if (peek() == '}')
                {
                    take();
                    return value;
                }
                while (true)
                {
                    skipWs();
                    Json key = parseValue();
                    skipWs();
                    take(); // ':'
                    value.object[key.string] = parseValue();
                    skipWs();
                    if (take() == '}')
                        return value;
                }
            }
            case '[':
            {
                take();
                value.type = Json::Type::Array;
                skipWs();
                if (peek() == ']')
                {
                    take();
                    return value;
                }
                while (true)
                {
                    value.array.push_back (parseValue());
                    skipWs();
                    if (take() == ']')
                        return value;
                }
            }
            case '"':
            {
                take();
                value.type = Json::Type::String;
                while (peek() != '"')
                {
                    char c = take();
                    if (c == '\\')
                    {
                        const char esc = take();
                        value.string += esc == 'n' ? '\n' : esc == 't' ? '\t' : esc;
                    }
                    else
                        value.string += c;
                }
                take();
                return value;
            }
            case 't':
                for (int i = 0; i < 4; ++i) take();
                value.type = Json::Type::Bool;
                value.boolean = true;
                return value;
            case 'f':
                for (int i = 0; i < 5; ++i) take();
                value.type = Json::Type::Bool;
                value.boolean = false;
                return value;
            case 'n':
                for (int i = 0; i < 4; ++i) take();
                return value;
            default:
            {
                value.type = Json::Type::Number;
                std::string digits;
                while (pos_ < text_.size()
                       && (isdigit (text_[pos_]) || text_[pos_] == '-' || text_[pos_] == '+' || text_[pos_] == '.'
                           || text_[pos_] == 'e' || text_[pos_] == 'E'))
                    digits += take();
                value.number = std::stod (digits);
                return value;
            }
        }
    }
};

// ---- SHA-256 (FIPS 180-4, compact implementation for fixture verification) ----
uint32_t rotr (uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }

std::string sha256 (const std::vector<uint8_t>& data)
{
    static const uint32_t k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };
    uint32_t h[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };

    std::vector<uint8_t> msg (data);
    const uint64_t bitLength = uint64_t (msg.size()) * 8;
    msg.push_back (0x80);
    while (msg.size() % 64 != 56)
        msg.push_back (0);
    for (int i = 7; i >= 0; --i)
        msg.push_back (uint8_t (bitLength >> (i * 8)));

    for (size_t block = 0; block < msg.size(); block += 64)
    {
        uint32_t w[64];
        for (int i = 0; i < 16; ++i)
            w[i] = (uint32_t (msg[block + size_t (i) * 4]) << 24) | (uint32_t (msg[block + size_t (i) * 4 + 1]) << 16)
                   | (uint32_t (msg[block + size_t (i) * 4 + 2]) << 8) | uint32_t (msg[block + size_t (i) * 4 + 3]);
        for (int i = 16; i < 64; ++i)
        {
            const uint32_t s0 = rotr (w[i - 15], 7) ^ rotr (w[i - 15], 18) ^ (w[i - 15] >> 3);
            const uint32_t s1 = rotr (w[i - 2], 17) ^ rotr (w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
        for (int i = 0; i < 64; ++i)
        {
            const uint32_t s1 = rotr (e, 6) ^ rotr (e, 11) ^ rotr (e, 25);
            const uint32_t ch = (e & f) ^ (~e & g);
            const uint32_t t1 = hh + s1 + ch + k[i] + w[i];
            const uint32_t s0 = rotr (a, 2) ^ rotr (a, 13) ^ rotr (a, 22);
            const uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            const uint32_t t2 = s0 + maj;
            hh = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }
        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
        h[5] += f;
        h[6] += g;
        h[7] += hh;
    }

    std::ostringstream hex;
    hex << std::hex;
    for (const uint32_t word : h)
        hex.width (8), hex.fill ('0'), hex << word;
    return hex.str();
}

std::vector<uint8_t> readFile (const std::filesystem::path& path)
{
    std::ifstream input (path, std::ios::binary);
    return std::vector<uint8_t> (std::istreambuf_iterator<char> (input), std::istreambuf_iterator<char>());
}

bool isHex (const std::string& text, size_t length)
{
    if (text.size() != length)
        return false;
    return std::all_of (text.begin(), text.end(), [] (char c) { return isxdigit (static_cast<unsigned char> (c)) != 0; });
}

bool isUtcTimestamp (const std::string& text)
{
    // YYYY-MM-DDTHH:MM:SSZ
    if (text.size() != 20 || text[4] != '-' || text[7] != '-' || text[10] != 'T' || text[13] != ':' || text[16] != ':' || text[19] != 'Z')
        return false;
    return std::all_of (text.begin(), text.end(), [] (char c) { return isdigit (static_cast<unsigned char> (c)) != 0 || c == '-' || c == 'T' || c == ':' || c == 'Z'; });
}
} // namespace

TEST_CASE ("T-023 golden manifest embeds full provenance and matches fixtures", "[frozen][golden]")
{
    const std::filesystem::path root = FAULTCOUNTER_REPO_ROOT;
    const auto manifestPath = root / "tests/golden/engine-golden-manifest.json";
    const auto manifestText = [&] {
        std::ifstream input (manifestPath);
        REQUIRE (input.good());
        return std::string (std::istreambuf_iterator<char> (input), std::istreambuf_iterator<char>());
    }();
    const Json manifest = JsonParser (manifestText).parse();
    REQUIRE (manifest.type == Json::Type::Object);

    // Provenance fields required by T-023.
    REQUIRE (manifest["commit"].type == Json::Type::String);
    REQUIRE (isHex (manifest["commit"].string, 40));
    REQUIRE (manifest["environmentLock"].type == Json::Type::String);
    REQUIRE (isHex (manifest["environmentLock"].string, 64));
    REQUIRE (manifest["generatedUtc"].type == Json::Type::String);
    REQUIRE (isUtcTimestamp (manifest["generatedUtc"].string));
    REQUIRE (manifest["seed"].type == Json::Type::Number);

    // Environment lock must equal the hash of the pinned environment file.
    const auto environmentHash = sha256 (readFile (root / "plan/ENVIRONMENT.md"));
    REQUIRE (manifest["environmentLock"].string == environmentHash);

    // Parameter state, sample rate, block schedule.
    REQUIRE (manifest["parameterState"].type == Json::Type::Object);
    REQUIRE (manifest["parameterState"]["sampleRate"].type == Json::Type::Number);
    REQUIRE (manifest["parameterState"]["bits"].type == Json::Type::Number);
    REQUIRE (manifest["parameterState"]["table"].type == Json::Type::Number);
    REQUIRE (manifest["parameterState"]["position"].type == Json::Type::String);
    REQUIRE (manifest["sampleRate"].type == Json::Type::Number);
    REQUIRE (manifest["blockSchedule"].type == Json::Type::Array);
    REQUIRE (! manifest["blockSchedule"].array.empty());
    int scheduledSamples = 0;
    for (const auto& block : manifest["blockSchedule"].array)
    {
        REQUIRE (block.type == Json::Type::Number);
        scheduledSamples += int (block.number);
    }
    REQUIRE (manifest["length"].type == Json::Type::Number);
    REQUIRE (scheduledSamples == int (manifest["length"].number));

    // Every embedded fixture hash and byte count matches the file on disk.
    REQUIRE (manifest["fixtures"].type == Json::Type::Object);
    for (const auto& [name, fixture] : manifest["fixtures"].object)
    {
        CAPTURE (name);
        REQUIRE (fixture["path"].type == Json::Type::String);
        REQUIRE (fixture["sha256"].type == Json::Type::String);
        REQUIRE (fixture["bytes"].type == Json::Type::Number);
        const auto bytes = readFile (root / fixture["path"].string);
        REQUIRE (bytes.size() == size_t (fixture["bytes"].number));
        REQUIRE (sha256 (bytes) == fixture["sha256"].string);
    }
}
