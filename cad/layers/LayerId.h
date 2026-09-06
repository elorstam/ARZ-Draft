#pragma once

#include <cstdint>

namespace arz::cad {

using LayerId = std::uint64_t;

inline constexpr LayerId InvalidLayerId = 0;
inline constexpr LayerId DefaultLayerId = 1;

}
