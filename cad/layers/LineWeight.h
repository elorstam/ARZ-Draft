#pragma once

#include <cstdint>

namespace arz::cad {

enum class LineWeight : std::int16_t {
    ByLayer = -1,
    ByBlock = -2,
    Default = -3,

    W005 = 5,
    W009 = 9,
    W013 = 13,
    W018 = 18,
    W025 = 25,
    W035 = 35,
    W050 = 50,
    W070 = 70,
    W100 = 100
};

[[nodiscard]] constexpr double lineWeightMillimeters(
    LineWeight value
) noexcept {
    switch (value) {
    case LineWeight::W005: return 0.05;
    case LineWeight::W009: return 0.09;
    case LineWeight::W013: return 0.13;
    case LineWeight::W018: return 0.18;
    case LineWeight::W025: return 0.25;
    case LineWeight::W035: return 0.35;
    case LineWeight::W050: return 0.50;
    case LineWeight::W070: return 0.70;
    case LineWeight::W100: return 1.00;
    default: return 0.0;
    }
}

}
