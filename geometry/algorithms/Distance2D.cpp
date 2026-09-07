#include "geometry/algorithms/Distance2D.h"

#include <algorithm>
#include <cmath>

namespace arz::geometry {

double distanceToSegment(
    Point2D point,
    Point2D start,
    Point2D end
) noexcept {
    const double dx = end.x - start.x;
    const double dy = end.y - start.y;
    const double lengthSquared = dx * dx + dy * dy;

    if (lengthSquared == 0.0) {
        return std::hypot(
            point.x - start.x,
            point.y - start.y
        );
    }

    const double projection =
        ((point.x - start.x) * dx
            + (point.y - start.y) * dy)
        / lengthSquared;
    const double parameter = std::clamp(
        projection,
        0.0,
        1.0
    );
    const Point2D closest{
        start.x + parameter * dx,
        start.y + parameter * dy
    };

    return std::hypot(
        point.x - closest.x,
        point.y - closest.y
    );
}

}
