#pragma once

#include <cstdint>

namespace arz::cad {

using LineTypeId = std::uint64_t;

inline constexpr LineTypeId InvalidLineTypeId = 0;
inline constexpr LineTypeId ContinuousLineTypeId = 1;

}
