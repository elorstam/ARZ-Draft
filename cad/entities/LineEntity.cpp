#include "cad/entities/LineEntity.h"

#include <algorithm>
#include <cmath>

namespace arz::cad {

LineEntity::LineEntity(
    arz::core::ObjectId id,
    LayerId layerId,
    arz::geometry::Point2D start,
    arz::geometry::Point2D end
)
    : CadEntity(id, layerId),
      start_(start),
      end_(end) {
}

EntityType LineEntity::entityType() const noexcept {
    return EntityType::Line;
}

arz::geometry::Point2D LineEntity::start() const noexcept {
    return start_;
}

arz::geometry::Point2D LineEntity::end() const noexcept {
    return end_;
}

void LineEntity::setStart(
    arz::geometry::Point2D point
) noexcept {
    start_ = point;
}

void LineEntity::setEnd(
    arz::geometry::Point2D point
) noexcept {
    end_ = point;
}

double LineEntity::length() const noexcept {
    const double dx = end_.x - start_.x;
    const double dy = end_.y - start_.y;

    return std::sqrt(
        dx * dx +
        dy * dy
    );
}

arz::geometry::BoundingBox2D
LineEntity::boundingBox() const noexcept {
    return {
        std::min(start_.x, end_.x),
        std::min(start_.y, end_.y),
        std::max(start_.x, end_.x),
        std::max(start_.y, end_.y)
    };
}

}
