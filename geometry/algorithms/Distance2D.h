#pragma once

#include "geometry/primitives/Point2D.h"

namespace arz::geometry {

[[nodiscard]] double distanceToSegment(
    Point2D point,
    Point2D start,
    Point2D end
) noexcept;

}
