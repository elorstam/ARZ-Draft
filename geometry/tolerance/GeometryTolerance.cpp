#include "geometry/tolerance/GeometryTolerance.h"

#include <cmath>

namespace arz::geometry {

bool GeometryTolerance::nearlyEqual(
    double a,
    double b
) const {
    return std::abs(a - b) <= linear;
}

const GeometryTolerance& defaultTolerance() {
    static const GeometryTolerance tolerance{};
    return tolerance;
}

}
