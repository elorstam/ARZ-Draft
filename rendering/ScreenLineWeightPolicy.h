#pragma once

#include "cad/layers/LineWeight.h"

namespace arz::rendering {

class ScreenLineWeightPolicy final {
public:
    static constexpr double DefaultDisplayPixelsPerMillimeter = 96.0 / 25.4;
    static constexpr double MinimumVisiblePixels = 1.0;

    [[nodiscard]] static double pixelWidth(
        arz::cad::LineWeight lineWeight,
        double displayPixelsPerMillimeter = DefaultDisplayPixelsPerMillimeter,
        double minimumVisiblePixels = MinimumVisiblePixels
    ) noexcept;
};

}
