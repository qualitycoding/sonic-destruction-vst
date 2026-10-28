#include "dsp/Tables.h"
#include <stdexcept>

namespace faultcounter
{
Tables Tables::loadProcedural (const std::filesystem::path&) { throw std::logic_error ("not implemented"); }
std::string Tables::sha256File (const std::filesystem::path&) { throw std::logic_error ("not implemented"); }
uint8_t Tables::lookup (int, uint8_t) const noexcept { return 0; }
int Tables::size() const noexcept { return 0; }
const std::vector<uint8_t>& Tables::data() const noexcept { return data_; }
} // namespace faultcounter
