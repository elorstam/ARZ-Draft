#include "cad/spatial/LinearSpatialIndex2D.h"

namespace arz::cad {

bool LinearSpatialIndex2D::insert(
    arz::core::ObjectId objectId,
    arz::geometry::BoundingBox2D bounds
) {
    if (objectId == arz::core::InvalidObjectId) {
        return false;
    }

    return entries_.emplace(
        objectId,
        bounds.normalized()
    ).second;
}

bool LinearSpatialIndex2D::update(
    arz::core::ObjectId objectId,
    arz::geometry::BoundingBox2D bounds
) {
    if (objectId == arz::core::InvalidObjectId) {
        return false;
    }

    const auto it = entries_.find(objectId);

    if (it == entries_.end()) {
        return false;
    }

    it->second = bounds.normalized();
    return true;
}

bool LinearSpatialIndex2D::remove(
    arz::core::ObjectId objectId
) {
    if (objectId == arz::core::InvalidObjectId) {
        return false;
    }

    return entries_.erase(objectId) == 1;
}

void LinearSpatialIndex2D::clear() noexcept {
    entries_.clear();
}

std::vector<arz::core::ObjectId>
LinearSpatialIndex2D::query(
    const arz::geometry::BoundingBox2D& bounds
) const {
    const auto queryBounds = bounds.normalized();
    std::vector<arz::core::ObjectId> result;

    for (const auto& [objectId, entryBounds] : entries_) {
        if (entryBounds.intersects(queryBounds)) {
            result.push_back(objectId);
        }
    }

    return result;
}

std::size_t LinearSpatialIndex2D::size() const noexcept {
    return entries_.size();
}

}
