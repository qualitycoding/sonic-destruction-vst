#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace faultcounter
{
inline constexpr int tableCount = 252;
inline constexpr int tableRowLength = 256;

class Tables
{
public:
    static Tables loadProcedural (const std::filesystem::path& path);
    static std::string sha256File (const std::filesystem::path& path);
    uint8_t lookup (int table, uint8_t code) const noexcept;
    int size() const noexcept;
    const std::vector<uint8_t>& data() const noexcept;

private:
    std::vector<uint8_t> data_;
};
} // namespace faultcounter
