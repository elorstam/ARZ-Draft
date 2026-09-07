#include "cad/snapping/LineEntitySnapProvider.h"

#include <algorithm>

#include "cad/entities/LineEntity.h"
#include "geometry/algorithms/Point2DOperations.h"

namespace arz::cad {

namespace {

bool isEnabled(
    std::span<const SnapType> enabledTypes,
    SnapType type
) {
    return std::ranges::find(enabledTypes, type)
        != enabledTypes.end();
}

}

void LineEntitySnapProvider::appendSnapPoints(
    const CadEntity& entity,
    std::span<const SnapType> enabledTypes,
    std::vector<SnapPoint>& points
) const {
    const auto* line = dynamic_cast<const LineEntity*>(&entity);

    if (line == nullptr) {
        return;
    }

    const auto start = line->start();
    const auto end = line->end();

    if (isEnabled(enabledTypes, SnapType::Endpoint)) {
        points.push_back({SnapType::Endpoint, start});

        if (end != start) {
            points.push_back({SnapType::Endpoint, end});
        }
    }

    if (isEnabled(enabledTypes, SnapType::Midpoint)) {
        points.push_back({
            SnapType::Midpoint,
            arz::geometry::midpoint(start, end)
        });
    }
}

}
