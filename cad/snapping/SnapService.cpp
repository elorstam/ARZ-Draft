#include "cad/snapping/SnapService.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <tuple>

#include "cad/entities/CadEntity.h"
#include "cad/snapping/IEntitySnapProvider.h"
#include "cad/spatial/ISpatialIndex2D.h"
#include "core/document/Document.h"
#include "geometry/algorithms/Point2DOperations.h"
#include "geometry/primitives/BoundingBox2D.h"

namespace arz::cad {

namespace {

int snapTypePriority(SnapType type) noexcept {
    switch (type) {
    case SnapType::Endpoint:
        return 0;
    case SnapType::Midpoint:
        return 1;
    }

    return 2;
}

bool rankedBefore(
    const SnapResult& first,
    const SnapResult& second
) noexcept {
    if (first.distance != second.distance) {
        return first.distance < second.distance;
    }

    const int firstPriority = snapTypePriority(first.type);
    const int secondPriority = snapTypePriority(second.type);

    if (firstPriority != secondPriority) {
        return firstPriority < secondPriority;
    }

    if (first.objectId != second.objectId) {
        return first.objectId < second.objectId;
    }

    if (first.point.x != second.point.x) {
        return first.point.x < second.point.x;
    }

    return first.point.y < second.point.y;
}

}

SnapService::SnapService(
    const arz::core::Document& document,
    const ISpatialIndex2D& spatialIndex,
    const IEntitySnapProvider& snapProvider
)
    : document_(document),
      spatialIndex_(spatialIndex),
      snapProvider_(snapProvider) {
}

std::vector<SnapResult> SnapService::candidates(
    arz::geometry::Point2D queryPoint,
    double tolerance,
    std::span<const SnapType> enabledTypes
) const {
    if (enabledTypes.empty()
        || tolerance < 0.0
        || !std::isfinite(tolerance)
        || !std::isfinite(queryPoint.x)
        || !std::isfinite(queryPoint.y)) {
        return {};
    }

    const arz::geometry::BoundingBox2D queryBounds{
        queryPoint.x - tolerance,
        queryPoint.y - tolerance,
        queryPoint.x + tolerance,
        queryPoint.y + tolerance
    };
    std::set<arz::core::ObjectId> visitedObjects;
    std::vector<SnapResult> results;

    for (const auto objectId : spatialIndex_.query(queryBounds)) {
        if (!visitedObjects.insert(objectId).second) {
            continue;
        }

        const auto* entity = dynamic_cast<const CadEntity*>(
            document_.object(objectId)
        );

        if (entity == nullptr) {
            continue;
        }

        std::vector<SnapPoint> points;
        snapProvider_.appendSnapPoints(
            *entity,
            enabledTypes,
            points
        );

        for (const auto& snapPoint : points) {
            const double snapDistance = arz::geometry::distance(
                queryPoint,
                snapPoint.point
            );

            if (std::isfinite(snapDistance)
                && snapDistance <= tolerance) {
                results.push_back({
                    objectId,
                    snapPoint.type,
                    snapPoint.point,
                    snapDistance
                });
            }
        }
    }

    std::ranges::sort(results, rankedBefore);
    std::set<std::tuple<arz::core::ObjectId, double, double>>
        uniquePoints;
    results.erase(
        std::remove_if(
            results.begin(),
            results.end(),
            [&uniquePoints](const SnapResult& result) {
                return !uniquePoints.emplace(
                    result.objectId,
                    result.point.x,
                    result.point.y
                ).second;
            }
        ),
        results.end()
    );
    return results;
}

std::optional<SnapResult> SnapService::bestSnap(
    arz::geometry::Point2D queryPoint,
    double tolerance,
    std::span<const SnapType> enabledTypes
) const {
    auto results = candidates(
        queryPoint,
        tolerance,
        enabledTypes
    );

    if (results.empty()) {
        return std::nullopt;
    }

    return results.front();
}

}
