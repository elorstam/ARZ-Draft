#include "cad/entities/ArcEntity.h"

#include <cmath>
#include <stdexcept>

#include "geometry/algorithms/Curve2D.h"

namespace arz::cad {

ArcEntity::ArcEntity(arz::core::ObjectId id, LayerId layerId,
                     arz::geometry::Point2D center, double radius,
                     double startAngle, double endAngle, bool counterClockwise)
    : CadEntity(id, layerId), center_(center), radius_(radius),
      startAngle_(arz::geometry::normalizeAngle(startAngle)),
      endAngle_(arz::geometry::normalizeAngle(endAngle)),
      counterClockwise_(counterClockwise) {
    if (!std::isfinite(radius_) || radius_ <= 0.0
        || !std::isfinite(startAngle) || !std::isfinite(endAngle))
        throw std::invalid_argument("Arc radius must be positive and angles finite");
}
EntityType ArcEntity::entityType() const noexcept { return EntityType::Arc; }
arz::geometry::Point2D ArcEntity::center() const noexcept { return center_; }
double ArcEntity::radius() const noexcept { return radius_; }
double ArcEntity::startAngle() const noexcept { return startAngle_; }
double ArcEntity::endAngle() const noexcept { return endAngle_; }
bool ArcEntity::counterClockwise() const noexcept { return counterClockwise_; }
double ArcEntity::sweepAngle() const noexcept {
    return arz::geometry::directedAngleSweep(startAngle_, endAngle_, counterClockwise_);
}
double ArcEntity::length() const noexcept { return radius_ * sweepAngle(); }
arz::geometry::Point2D ArcEntity::startPoint() const noexcept {
    return arz::geometry::pointOnCircle(center_, radius_, startAngle_);
}
arz::geometry::Point2D ArcEntity::endPoint() const noexcept {
    return arz::geometry::pointOnCircle(center_, radius_, endAngle_);
}
arz::geometry::BoundingBox2D ArcEntity::boundingBox() const noexcept {
    return arz::geometry::arcBounds(center_, radius_, startAngle_, endAngle_, counterClockwise_);
}

}
