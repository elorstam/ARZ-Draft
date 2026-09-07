#include "cad/selection/SelectionService.h"

#include <cmath>
#include <set>

#include "cad/entities/CadEntity.h"
#include "cad/selection/IEntityPickRefiner.h"
#include "cad/spatial/ISpatialIndex2D.h"
#include "core/document/Document.h"
#include "geometry/primitives/BoundingBox2D.h"

namespace arz::cad {

namespace {

arz::geometry::BoundingBox2D windowBounds(
    arz::geometry::Point2D first,
    arz::geometry::Point2D second
) {
    return arz::geometry::BoundingBox2D{
        first.x,
        first.y,
        second.x,
        second.y
    }.normalized();
}

std::vector<arz::core::ObjectId> toVector(
    const std::set<arz::core::ObjectId>& objectIds
) {
    return {objectIds.begin(), objectIds.end()};
}

}

SelectionService::SelectionService(
    const arz::core::Document& document,
    const ISpatialIndex2D& spatialIndex,
    const IEntityPickRefiner& pickRefiner
)
    : document_(document),
      spatialIndex_(spatialIndex),
      pickRefiner_(pickRefiner) {
}

std::vector<arz::core::ObjectId>
SelectionService::pointPick(
    arz::geometry::Point2D point,
    double tolerance
) const {
    if (tolerance < 0.0 || !std::isfinite(tolerance)) {
        return {};
    }

    const arz::geometry::BoundingBox2D queryBounds{
        point.x - tolerance,
        point.y - tolerance,
        point.x + tolerance,
        point.y + tolerance
    };
    std::set<arz::core::ObjectId> selected;

    for (const auto objectId : spatialIndex_.query(queryBounds)) {
        const auto* entity = dynamic_cast<const CadEntity*>(
            document_.object(objectId)
        );

        if (entity != nullptr
            && pickRefiner_.hit(*entity, point, tolerance)) {
            selected.insert(objectId);
        }
    }

    return toVector(selected);
}

std::vector<arz::core::ObjectId>
SelectionService::crossingWindow(
    arz::geometry::Point2D first,
    arz::geometry::Point2D second
) const {
    const auto window = windowBounds(first, second);
    std::set<arz::core::ObjectId> selected;

    for (const auto objectId : spatialIndex_.query(window)) {
        const auto* entity = dynamic_cast<const CadEntity*>(
            document_.object(objectId)
        );

        if (entity != nullptr
            && pickRefiner_.intersects(*entity, window)) {
            selected.insert(objectId);
        }
    }

    return toVector(selected);
}

std::vector<arz::core::ObjectId>
SelectionService::containedWindow(
    arz::geometry::Point2D first,
    arz::geometry::Point2D second
) const {
    const auto window = windowBounds(first, second);
    std::set<arz::core::ObjectId> selected;

    for (const auto objectId : spatialIndex_.query(window)) {
        const auto* entity = dynamic_cast<const CadEntity*>(
            document_.object(objectId)
        );

        if (entity != nullptr
            && pickRefiner_.contained(*entity, window)) {
            selected.insert(objectId);
        }
    }

    return toVector(selected);
}

}
