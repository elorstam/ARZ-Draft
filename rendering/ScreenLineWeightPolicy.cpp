#include "rendering/ScreenLineWeightPolicy.h"

#include <algorithm>
#include <cmath>

namespace arz::rendering {

double ScreenLineWeightPolicy::pixelWidth(
    arz::cad::LineWeight lineWeight,
    double displayPixelsPerMillimeter,
    double minimumVisiblePixels
) noexcept {
    if (!std::isfinite(displayPixelsPerMillimeter)
        || displayPixelsPerMillimeter <= 0.0) {
        displayPixelsPerMillimeter = DefaultDisplayPixelsPerMillimeter;
    }
    if (!std::isfinite(minimumVisiblePixels) || minimumVisiblePixels <= 0.0) {
        minimumVisiblePixels = MinimumVisiblePixels;
    }
    double millimeters = arz::cad::lineWeightMillimeters(lineWeight);
    if (millimeters <= 0.0) millimeters = 0.25;
    return std::max(minimumVisiblePixels, millimeters * displayPixelsPerMillimeter);
}

}
