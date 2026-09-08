#include "interaction/point/PointAcquisition.h"

#include <cmath>

namespace arz::interaction {

ResolvedCadPoint PointAcquisition::resolve(
    const PointAcquisitionContext& context
) const noexcept {
    ResolvedCadPoint result;
    result.rawPoint = context.rawPoint;
    result.point = context.rawPoint;

    // A positive object snap is an explicit geometric target and takes
    // precedence over Ortho. This also guarantees that a displayed marker is
    // always the exact point used by preview and commit.
    if (context.snap) {
        result.snappedPoint = context.snap->point;
        result.point = context.snap->point;
        result.snap = context.snap;
        return result;
    }

    if (context.orthoEnabled && context.constraintOrigin) {
        const auto origin = *context.constraintOrigin;
        const double dx = context.rawPoint.x - origin.x;
        const double dy = context.rawPoint.y - origin.y;
        result.constrainedPoint = std::abs(dx) >= std::abs(dy)
            ? arz::geometry::Point2D{context.rawPoint.x, origin.y}
            : arz::geometry::Point2D{origin.x, context.rawPoint.y};
        result.point = *result.constrainedPoint;
        result.orthoConstrained = result.point != context.rawPoint;
    }

    return result;
}

}
