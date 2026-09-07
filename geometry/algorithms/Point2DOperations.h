#pragma once

#include <cmath>
#include <numeric>

#include "geometry/primitives/Point2D.h"

namespace arz::geometry {

[[nodiscard]] inline double distance(
    Point2D first,
    Point2D second
) noexcept {
    return std::hypot(
        second.x - first.x,
        second.y - first.y
    );
}

[[nodiscard]] inline Point2D midpoint(
    Point2D first,
    Point2D second
) noexcept {
    return {
        std::midpoint(first.x, second.x),
        std::midpoint(first.y, second.y)
    };
}

}
