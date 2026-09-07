#include "cad/selection/LineEntityPickRefiner.h"

#include "cad/entities/LineEntity.h"
#include "geometry/algorithms/Distance2D.h"

namespace arz::cad {

bool LineEntityPickRefiner::hit(
    const CadEntity& entity,
    arz::geometry::Point2D point,
    double tolerance
) const noexcept {
    const auto* line = dynamic_cast<const LineEntity*>(&entity);

    if (line == nullptr || tolerance < 0.0) {
        return false;
    }

    return arz::geometry::distanceToSegment(
        point,
        line->start(),
        line->end()
    ) <= tolerance;
}

}
